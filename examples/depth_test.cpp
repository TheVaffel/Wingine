#include <Winval.hpp>

#include "../include/Wingine.hpp"
#include "../include/WgUtils.hpp"

#include <spurv.hpp>

#include <chrono>

int main() {

    const int width = 1280, height = 720;
    Winval win(width, height);

    win.setTitle("Wingine - Depth Example");
    wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());

    const int num_points = 7;
    const int num_triangles = 3;

    float positions[num_points * 4] = {
        1.0f, -1.0f, -2.5f, 1.0f,
        -1.0f, -1.0f, -2.5f, 1.0f,
        0.0f, 1.0f, -2.5f, 1.0f,

        // Plane
        -2.f, -1.f, -6.5f, 1.0f,
        2.f, -1.f, -6.5f, 1.0f,
        -2.f, -1.f, 1.5f, 1.0f,
        2.f, -1.f, 1.5f, 1.0f
    };

    float colors[num_points * 4] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    };

    uint32_t indices[num_triangles * 3] = {
        0, 1, 2,
        3, 4, 5,
        5, 4, 6
    };

    wg::VertexBufferPtr<float> position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, 0, num_points * 4);

    wg::VertexBufferPtr<float> color_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    color_buffer->set(colors, 0, num_points * 4);

    wg::IndexBufferPtr index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
    index_buffer->set(indices, 0, num_triangles * 3);


    wg::UniformChainPtr<falg::Mat4> cameraUniform = wing.createUniformChain<falg::Mat4>();
    wg::UniformChainPtr<falg::Mat4> light_uniform = wing.createUniformChain<falg::Mat4>();

    std::vector<uint64_t> resourceSetLayout = { wg::resUniform | wg::shaVertex };

    wg::ResourceSetChainPtr resourceSet = wing.createResourceSetChain(resourceSetLayout, cameraUniform);

    wg::ResourceSetChainPtr lightSet = wing.createResourceSetChain(resourceSetLayout, light_uniform);

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
    {
        wg::VertexAttribDesc(0, wg::ComponentType::Float32, 4, 4 * sizeof(float), 0),
        wg::VertexAttribDesc(1, wg::ComponentType::Float32, 4, 4 * sizeof(float), 0)
    };

    // Some texture size
    const uint32_t shadow_buffer_width = 2000,
        shadow_buffer_height = 2000;

    wg::FramebufferTextureChainPtr shadow_framebuffer_chain = wing.createFramebufferTextureChain(
        shadow_buffer_width, shadow_buffer_height, true);

    std::vector<uint64_t> lightTextureSetLayout = {wg::resTexture | wg::shaFragment,
                                                   wg::resUniform | wg::shaFragment};

    wg::ResourceSetChainPtr lightTextureSet = wing.createResourceSetChain(lightTextureSetLayout,
                                                                          shadow_framebuffer_chain, light_uniform);

    std::vector<uint32_t> depth_vertex_shader;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec4_s> shader;
        SPointerVar<SStruct<SDecoration::BLOCK, mat4_s>, STORAGE_UNIFORM>& trans_bind = shader.uniformBinding<mat4_s>(0, 0);
        mat4_v trans = trans_bind.member<0>().load();

        vec4_v pos = shader.input<0>();

        vec4_v transformed_pos = trans * pos;

        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(depth_vertex_shader);

    }

    wg::ShaderPtr depth_shader = wing.createShader(wg::ShaderStage::Vertex, depth_vertex_shader);

    wg::BasicPipelineSetup shadow_pipeline_setup;
    shadow_pipeline_setup.setWidth(shadow_buffer_width)
        .setHeight(shadow_buffer_height)
        .setDepthOnly(true);
    wg::PipelinePtr depth_pipeline = wing.createBasicPipeline({vertAttrDesc[0]},
                                                              {resourceSetLayout},
                                                              {depth_shader},
                                                              shadow_pipeline_setup);

    wg::BasicDrawPassSettings shadow_draw_pass_settings;
    shadow_draw_pass_settings.render_pass_settings
        .setDepthOnly()
        .setFinalizeAsTexture(true);

    wg::DrawPassPtr shadow_draw_pass = wing.createBasicDrawPass(depth_pipeline, shadow_draw_pass_settings);

    shadow_draw_pass->getCommandChain().startRecording(shadow_framebuffer_chain);
    shadow_draw_pass->getCommandChain().recordDraw({ position_buffer, color_buffer }, index_buffer, { lightSet });
    shadow_draw_pass->getCommandChain().endRecording();

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec4_s, vec4_s> shader;
        vec4_v s_pos = shader.input<0>();
        vec4_v s_col = shader.input<1>();

        auto trans_bind = shader.uniformBinding<mat4_s>(0, 0);
        mat4_v trans = trans_bind.member<0>().load();

        vec4_v transformed_pos = trans * s_pos;
        vec4_v world_pos = s_pos; // Should be model transformed, but doesn't have one in this example

        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(vertex_spirv, s_col, world_pos);
    }

    wg::ShaderPtr vertex_shader = wing.createShader(wg::ShaderStage::Vertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_FRAGMENT, vec4_s, vec4_s> shader;
        vec4_v in_col = shader.input<0>();
        vec4_v in_wpos = shader.input<1>();

        texture2D_v shadow_tex = shader.uniformConstant<texture2D_s>(1, 0).load();
        auto shadow_trans_bind = shader.uniformBinding<mat4_s>(1, 1);
        mat4_v shadow_trans = shadow_trans_bind.member<0>().load();

        vec4_v light_pos = shadow_trans * in_wpos;

        vec2_v divided_lpos = vec2_s::cons(light_pos[0] / light_pos[3],
                                           light_pos[1] / light_pos[3]);

        vec2_v light_pos2d = (divided_lpos +
                              vec2_s::cons(1.0, 1.0)) * 0.5;

        float_v lookupval = select(max(fabs(divided_lpos[0]),
                                       fabs(divided_lpos[1])) <= 1.0,
                                   shadow_tex[light_pos2d][0], 1.0f);

        float_v intensity = select(lookupval + 1e-5 >= light_pos[2] / light_pos[3],
                                   1.f, 0.2f);

        vec4_v mul = intensity * in_col;

        vec4_v res = vec4_s::cons(mul[0], mul[1], mul[2], 1.0f);

        shader.compile(fragment_spirv, res);
    }

    wg::ShaderPtr fragment_shader = wing.createShader(wg::ShaderStage::Fragment, fragment_spirv);

    wg::PipelinePtr pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                            {resourceSetLayout, lightTextureSetLayout},
                            {vertex_shader, fragment_shader});

    wg::BasicDrawPassSettings draw_pass_settings;
    draw_pass_settings.render_pass_settings.setShouldClear(true);

    wg::DrawPassPtr real_draw_pass = wing.createBasicDrawPass(pipeline, draw_pass_settings);

    real_draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    real_draw_pass->getCommandChain().recordDraw({ position_buffer, color_buffer }, index_buffer, { resourceSet, lightTextureSet });
    real_draw_pass->getCommandChain().endRecording();

    shadow_draw_pass->getSemaphores().setWaitSemaphores({ wing.createAndAddImageReadySemaphore() });
    real_draw_pass->getSemaphores().setWaitSemaphores({ shadow_draw_pass->getSemaphores().createOnFinishSemaphore() });
    wing.setPresentWaitForSemaphores({ real_draw_pass->getSemaphores().createOnFinishSemaphore() });

    wgut::Camera camera(F_PI / 3.f, 9.0 / 16.0, 0.1f, 50.0f);
    camera.setLookAt(falg::Vec3(6.0f, 4.0f, 6.0f),
                     falg::Vec3(0.0f, 0.0f, -2.5f),
                     falg::Vec3(0.0f, 1.0f, 0.0f));

    wgut::Camera light_camera(F_PI / 3.f, 1.0f, 0.1f, 100.0f);

    float theta = 0;
    int fps = 0;

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    while (win.isOpen()) {

        theta += 0.02f;
        theta = fmodf(theta, 2.f * F_PI);

        light_camera.setLookAt(falg::Vec3(0.0f + 2.0f * cos(theta), 3.0f + 1.0f * sin(theta), 2.0f * sin(theta) + 3.0),
                               falg::Vec3(0.0f, 0.0f, -2.5f),
                               falg::Vec3(0.0f, 1.0f, 0.0f));


        // We need to await the draw passes here, because the draws we scheduled 3 (or 2) iterations
        // ago may still not be finished, and setting the uniform will mess them up real good if that's the case
        shadow_draw_pass->awaitCurrentCommand();
        real_draw_pass->awaitCurrentCommand();

        light_uniform->setCurrent(light_camera.getRenderMatrix());

        falg::Mat4 renderMatrix = camera.getRenderMatrix();

        shadow_draw_pass->render();

        cameraUniform->setCurrent(renderMatrix);

        real_draw_pass->render();

        wing.present();

        auto now = clock.now();

        double duration = std::chrono::duration<double>(now - start).count();
        fps++;
        if(duration >= 1.0f) {
            std::cout << "FPS = " << fps << std::endl;
            fps = 0;
            start = now;
        }

        win.flushEvents();
        if(win.isKeyPressed(WK_ESC)) {
            break;
        }
    }

    wing.waitIdle();
}
