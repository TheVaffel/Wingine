#include <Winval.hpp>

#include "../include/Wingine.hpp"
#include "../include/WgUtils.hpp"

#include <spurv.hpp>

#include <chrono>
#include <random>

int main() {
    int width = 800, height = 800;
    Winval win(width, height);
    wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());

    width = wing.getWindowWidth();
    height = wing.getWindowHeight();

    wgut::Model model = wgut::Model::fromFile(wing, "../models/teapot.obj",
                                              {wgut::VertexAttribute::Position,
                                               wgut::VertexAttribute::Normal});

    std::default_random_engine generator;
    std::uniform_real_distribution<float> dist(-50.0, 50.0);
    std::uniform_real_distribution<float> dist1(0.0, 1.0);

    const int num_instances = 20000;
    std::vector<float> offsets = std::vector<float>(3 * num_instances);
    std::vector<float> colors = std::vector<float>(3 * num_instances);
    std::vector<falg::Mat4> inst_mats = std::vector<falg::Mat4>(num_instances);

    for(int i = 0; i < num_instances; i++) {
        falg::Vec3 off;
        do {
            off = falg::Vec3(dist(generator), dist(generator), dist(generator));
        } while(off.sqNorm() <= 5.0f * 5.0f);

        for(int j = 0; j < 3; j++) {
            offsets[3 * i + j] = off[j];
            colors[3 * i + j] = dist1(generator);
        }

        float phi = 2 * F_PI * dist1(generator) - F_PI;
        float theta = F_PI * dist1(generator) - F_PI / 2;

        inst_mats[i] = falg::Mat4(falg::FLATALG_MATRIX_ROTATION_Y, phi) *
            falg::Mat4(falg::FLATALG_MATRIX_ROTATION_X, theta);
    }

    wg::VertexBufferPtr<float> offset_buffer = wing.createVertexBuffer<float>(3 * num_instances);
                                                    offset_buffer->set(offsets.data(), 0, 3 * num_instances);

    wg::VertexBufferPtr<float> color_buffer = wing.createVertexBuffer<float>(3 * num_instances);
                                                    color_buffer->set(colors.data(), 0, 3 * num_instances);

    wg::StorageBufferPtr<falg::Mat4> storage_buffer = wing.createStorageBuffer<falg::Mat4>(num_instances);
    storage_buffer->set(inst_mats.data(), 0, num_instances);

    struct CamColl {
        falg::Mat4 cam, view;
    };

    wg::UniformChainPtr<CamColl> cameraUniform = wing.createUniformChain<CamColl>();

    // Positions, color, offset
    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {wg::VertexAttribDesc(0, // Binding no.
                                                                wg::ComponentType::Float32, // Component type
                                                                3, // Number of elements
                                                                3 * sizeof(float), // Stride (in bytes)
                                                                0), // Offset (bytes)
        wg::VertexAttribDesc(1, wg::ComponentType::Float32, 3, 3 * sizeof(float), 0), // Normal
        // Color
        wg::VertexAttribDesc(2, wg::ComponentType::Float32, 3, 3 * sizeof(float), 0, true), // true - per_instance
        // Offset
        wg::VertexAttribDesc(3, wg::ComponentType::Float32, 3, 3 * sizeof(float), 0, true)
    };

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec3_s, vec3_s, vec3_s, vec3_s> shader;
        vec3_v s_pos = shader.input<0>();
        vec3_v s_norm = shader.input<1>();
        vec3_v s_off = shader.input<2>();
        vec3_v s_col = shader.input<3>();

        auto trans_bind = shader.uniformBinding<mat4_s, mat4_s>(0, 0);
        mat4_v trans = trans_bind.member<0>().load();
        mat4_v view = trans_bind.member<1>().load();

        /* vec4_v v0 = vec4_s::cons(1.0f, 0.0f, 0.0f, 0.0f);
           vec4_v v1 = vec4_s::cons(0.0f, 0.5f, 0.0f, 0.0f);
           vec4_v v2 = vec4_s::cons(0.0f, 0.0f, 0.5f, 0.0f);
           vec4_v v3 = vec4_s::cons(0.0f, 0.0f, 0.0f, 1.0f); */

        // Just to demonstrate matrix construction for columns
        // mat4_v scale = mat4_s::cons(v0, v1, v2, v3);

        auto inst_trans_bind = shader.storageBuffer<mat4_sarr_s>(0, 1);
        auto mat_array = inst_trans_bind.member<0>();

        uint_v instance_id = shader.getBuiltin<BUILTIN_INSTANCE_INDEX>();

        mat4_v this_mat = mat_array[instance_id].load();

        vec4_v het = vec4_s::cons(s_pos, 1.0f);
        vec4_v het_off = vec4_s::cons(s_off, 0.0f);
        vec4_v world_pos = het_off + this_mat * het;

        vec4_v transformed_pos = trans * world_pos;
        vec4_v norm_view = view * this_mat * vec4_s::cons(s_norm, 0.0f);
        vec4_v pos_view = view * world_pos;

        vec3_v trunc_norm = vec3_s::cons(norm_view[0], norm_view[1], norm_view[2]);

        // vec4_v hcol = vec4_s::cons(1.0f, 0.0f, 0.0f, 1.0f);

        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(vertex_spirv, s_col, trunc_norm, pos_view);

        // prettyprint(vertex_spirv);
    }

    wg::ShaderPtr vertex_shader = wing.createShader(wg::ShaderStage::Vertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_FRAGMENT, vec3_s, vec3_s, vec4_s> shader;
        vec3_v in_col = shader.input<0>();
        vec3_v s_norm = normalize(shader.input<1>());
        vec4_v s_pos = shader.input<2>();


        vec3_v sun_dir = normalize(vec3_s::cons(1.0f, -1.0f, 0.5f));

        vec3_v pos_cam_dir = -normalize(vec3_s::cons(s_pos[0], s_pos[1], s_pos[2]));

        float_v ldir_diff = max(dot(reflect(sun_dir, s_norm), pos_cam_dir), 0.0f);

        float_v dt = max(-dot(s_norm, sun_dir), 0.0f);

        float_v intensity = 0.1f + dt + pow(ldir_diff, 20.0f);

        vec4_v out_col = vec4_s::cons(intensity * in_col, 1.0f);

        shader.compile(fragment_spirv, out_col);

        // prettyprint(fragment_spirv);
    }

    wg::ShaderPtr fragment_shader = wing.createShader(wg::ShaderStage::Fragment, fragment_spirv);

    wg::PipelinePtr pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                            {vertex_shader, fragment_shader});


    wg::BasicDrawPassSettings draw_pass_settings;
    draw_pass_settings.render_pass_settings.setShouldClear(true);
    wg::DrawPassPtr draw_pass = wing.createBasicDrawPass(pipeline, draw_pass_settings);

    wgut::Camera camera(F_PI / 3.f, (float)height / (float)width, 0.01f, 1000.0f);
    float phi = 0.0;

    draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    draw_pass->getCommandChain().recordBindResourceSet(0, {{ 0, cameraUniform }, { 1, wing.createResourceChain(storage_buffer) }});
    draw_pass->getCommandChain().recordDraw({model.getVertexBuffers()[0], model.getVertexBuffers()[1], offset_buffer, color_buffer},
                                            model.getIndexBuffer(), num_instances);
    draw_pass->getCommandChain().endRecording();

    CamColl camcoll;

    int fps = 0;

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    draw_pass->getSemaphores().setWaitSemaphores({ wing.createAndAddImageReadySemaphore() });
    wing.setPresentWaitForSemaphores({ draw_pass->getSemaphores().createOnFinishSemaphore() });

    while (win.isOpen()) {

        phi += 0.003;

        camera.setLookAt(3.0f * falg::Vec3(sin(phi), 1.0f, cos(phi)),
                         falg::Vec3(0.0f, 0.0f, 0.0f),
                         falg::Vec3(0.0f, 1.0f, 0.0f));

        falg::Mat4 renderMatrix = camera.getRenderMatrix();
        falg::Mat4 rViewMatrix = ~camera.getViewMatrix();

        camcoll = {renderMatrix, rViewMatrix};

        draw_pass->awaitCurrentCommand();

        cameraUniform->setCurrent(camcoll);

        draw_pass->render();

        wing.present();

        auto now = clock.now();

        double duration = std::chrono::duration<double>(now - start).count();
        fps++;
        if(duration >= 1.0f) {
            std::cout << "FPS = " << fps << std::endl;
            fps = 0;
            start = now;
        }

        // win.sleepMilliseconds(30);

        win.flushEvents();
        if(win.isKeyPressed(WK_ESC)) {
            break;
        }
    }

    wing.waitIdle();
}
