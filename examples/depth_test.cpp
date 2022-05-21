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

    wg::VertexBuffer<float>* position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, num_points * 4);

    wg::VertexBuffer<float>* color_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    color_buffer->set(colors, num_points * 4);

    wg::IndexBuffer* index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
    index_buffer->set(indices, num_triangles * 3);


    wg::Uniform<falg::Mat4> cameraUniform = wing.createUniform<falg::Mat4>();
    wg::Uniform<falg::Mat4> lightUniform = wing.createUniform<falg::Mat4>();

    // Initialize resource set layout

    std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};

    wg::ResourceSet* resourceSet = wing.createResourceSet(resourceSetLayout);
    resourceSet->set({cameraUniform});

    wg::ResourceSet* lightSet = wing.createResourceSet(resourceSetLayout);
    lightSet->set({lightUniform});

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
                                            0, // Binding no.
                                            4, // Number of elements
                                            4 * sizeof(float), // Stride (in bytes)
                                            0}, // Offset (bytes)
                                           {wg::tFloat32, 1, 4, 4 * sizeof(float), 0}};

    // Some random size
    const uint32_t shadow_buffer_width = 2000,
        shadow_buffer_height = 2000;

    wg::BasicTextureSetup tex_setup;
    tex_setup.setDepthOnly(true);
    wg::TexturePtr shadow_texture = wing.createBasicTexture(shadow_buffer_width,
                                                            shadow_buffer_height, tex_setup);

    // Initialize texture set layout

    std::vector<uint64_t> lightTextureSetLayout = {wg::resTexture | wg::shaFragment,
                                                   wg::resUniform | wg::shaFragment};

    wg::ResourceSet* lightTextureSet = wing.createResourceSet(lightTextureSetLayout);
    lightTextureSet->set({shadow_texture, lightUniform });

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

    wg::Shader* depth_shader = wing.createShader(wg::shaVertex, depth_vertex_shader);

    wg::PipelineSetup shadow_pipeline_setup;
    shadow_pipeline_setup.setWidth(shadow_buffer_width)
        .setHeight(shadow_buffer_height)
        .setDepthOnly(true);
    wg::Pipeline* depth_pipeline = wing.createPipeline({vertAttrDesc[0]},
                                                       {resourceSetLayout},
                                                       {depth_shader},
                                                       shadow_pipeline_setup);


    wg::BasicDrawPassSettings shadow_draw_pass_settings;
    shadow_draw_pass_settings.setDepthOnly(true);

    wg::FramebufferChain shadow_framebuffer_chain =
        wing.createFramebufferChain(shadow_buffer_width, shadow_buffer_height, true);

    wg::DrawPassPtr shadow_draw_pass = wing.createBasicDrawPass(depth_pipeline, shadow_draw_pass_settings);

    shadow_draw_pass->startRecording(shadow_framebuffer_chain);
    shadow_draw_pass->recordDraw({ position_buffer, color_buffer }, index_buffer, { lightSet });
    shadow_draw_pass->endRecording();

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec4_s, vec4_s> shader;
        vec4_v s_pos = shader.input<0>();
        vec4_v s_col = shader.input<1>();

        // SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
        auto trans_bind = shader.uniformBinding<mat4_s>(0, 0);
        mat4_v trans = trans_bind.member<0>().load();

        vec4_v transformed_pos = trans * s_pos;
        vec4_v world_pos = s_pos; // Should be model transformed, but doesn't have one in this example

        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(vertex_spirv, s_col, world_pos);

        // SUtils::binaryPrettyPrint(vertex_spirv);
    }

    wg::Shader* vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);


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

        float_v intensity = select(lookupval + ( 1e-5) >= light_pos[2] / light_pos[3],
                                   1.f, 0.2f);

        // float_v intensity = lookupval;
        vec4_v mul = intensity * in_col;

        vec4_v res = vec4_s::cons(mul[0], mul[1], mul[2], 1.0f);

        shader.compile(fragment_spirv, res);
    }


    wg::Shader* fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);

    wg::Pipeline* pipeline = wing.
        createPipeline(vertAttrDesc,
                       {resourceSetLayout, lightTextureSetLayout},
                       {vertex_shader, fragment_shader});


    // wg::RenderFamily* family = wing.createRenderFamily(pipeline, true);

    wg::BasicDrawPassSettings draw_pass_settings;
    draw_pass_settings.setShouldClear(true);

    wg::DrawPassPtr real_draw_pass = wing.createBasicDrawPass(pipeline, draw_pass_settings);

    real_draw_pass->startRecording(wing.getDefaultFramebufferChain());
    real_draw_pass->recordDraw({ position_buffer, color_buffer }, index_buffer, { resourceSet, lightTextureSet });
    real_draw_pass->endRecording();

    wg::ImageCopierPtr image_copier = wing.createImageCopier();

    shadow_draw_pass->getSemaphores().setWaitSemaphores({ wing.createAndAddImageReadySemaphore() });
    image_copier->getSemaphores().setWaitSemaphores({ shadow_draw_pass->getSemaphores().createOnFinishSemaphore() });
    real_draw_pass->getSemaphores().setWaitSemaphores({ image_copier->getSemaphores().createOnFinishSemaphore() });
    wing.setPresentWaitForSemaphores({ real_draw_pass->getSemaphores().createOnFinishSemaphore() });

    wgut::Camera camera(F_PI / 3.f, 9.0 / 16.0, 0.01f, 100.0f);
    camera.setLookAt(falg::Vec3(6.0f, 4.0f, 6.0f),
                     falg::Vec3(0.0f, 0.0f, -2.5f),
                     falg::Vec3(0.0f, 1.0f, 0.0f));

    wgut::Camera light_camera(F_PI / 3.f, 1.0f, 0.01f, 100.0f);
    light_camera.setLookAt(falg::Vec3(-1.0f, 3.0f, -1.0f),
                           falg::Vec3(0.0f, 0.0f, -2.5f),
                           falg::Vec3(0.0f, 1.0f, 0.0f));

    float theta = 0;
    int fps = 0;

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();
    while (win.isOpen()) {
        theta += 0.02f;
        theta = fmodf(theta, 2.f * F_PI);

        light_camera.setLookAt(falg::Vec3(2.0f * cos(theta), 3.0f, 2.0f * sin(theta)),
                               falg::Vec3(0.0f, 0.0f, -2.5f),
                               falg::Vec3(0.0f, 1.0f, 0.0f));

        lightUniform->set(light_camera.getRenderMatrix());

        falg::Mat4 renderMatrix = camera.getRenderMatrix();

        shadow_draw_pass->render();

        image_copier->recordCopyImage(shadow_framebuffer_chain->getCurrentFramebuffer().getDepthImage(),
                                      *shadow_texture);
        image_copier->runCopy();

        cameraUniform->set(renderMatrix);

        real_draw_pass->render();

        wing.present();

        win.sleepMilliseconds(30);

        auto now = clock.now();

        double duration = std::chrono::duration<double>(now - start).count();
        fps++;
        if(duration >= 1.0f) {
            std::cout << "FPS = " << fps << std::endl;
            fps = 0;
            start = now;
        }

        // Main reason for the wait here is to ensure that setting the uniforms
        // for the next frame does not interfere with unfinished drawing
        win.flushEvents();
        if(win.isKeyPressed(WK_ESC)) {
            break;
        }

        shadow_framebuffer_chain->swapFramebuffer();

    }

    wing.waitIdle();

    wing.destroy(depth_shader);
    wing.destroy(depth_pipeline);

    wing.destroy(vertex_shader);
    wing.destroy(fragment_shader);

    wing.destroy(pipeline);

    wing.destroy(position_buffer);
    wing.destroy(color_buffer);
    wing.destroy(index_buffer);
}
