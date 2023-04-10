#include <Winval.hpp>

#include "../include/Wingine.hpp"
#include "../include/WgUtils.hpp"

#include <spurv.hpp>

int main() {
    const int width = 800, height = 800;
    Winval win(width, height);
    wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());

    win.setPointerVisible(false);

    const int num_points = 4;
    const int num_triangles = 2;

    float positions[num_points * 4] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f
    };

    uint32_t indices[num_triangles * 3] = {
        0, 1, 2,
        1, 3, 2
    };

    wg::VertexBufferPtr<float> position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, 0, num_points * 4);

    wg::IndexBufferPtr index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
    index_buffer->set(indices, 0, num_triangles * 3);

    wgut::Model model({position_buffer}, index_buffer);

    wg::UniformChainPtr<float> time_uniform = wing.createUniformChain<float>();
    wg::ResourceSetChainPtr time_set = wing.createResourceSetChain(time_uniform);

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc>
        {
            wg::VertexAttribDesc(0, wg::ComponentType::Float32, 4, 4 * sizeof(float), 0)
        };


    float scale = 0.001f;
    float offx = -0.77568377f;
    float offy = 0.13646737f;

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec4_s> shader;
        vec4_v s_pos = shader.input<0>();
        uint_v vi = shader.getBuiltin<BUILTIN_VERTEX_INDEX>();
        float_v time = shader.uniformBinding<float_s>(0, 0).member<0>().load();

        // Compute corners, (-1, -1) to (1, 1)
        float_v pv0 = cast<float_s>(vi % 2) * 2.f - 1.f;
        float_v pv1 = cast<float_s>(vi / 2) * 2.f - 1.f;

        vec2_v coord = vec2_s::cons(pv0, pv1) * exp((- time - 1) / 2 * 10) + vec2_s::cons(offx, offy);

        shader.setBuiltin<BUILTIN_POSITION>(s_pos);
        shader.compile(vertex_spirv, coord);

    }

    wg::ShaderPtr vertex_shader = wing.createShader(wg::ShaderStage::Vertex, vertex_spirv);

    int mandelbrot_iterations = 1000;
    float max_rad = 4.f;

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_FRAGMENT, vec2_s> shader;

        vec2_v coord = shader.input<0>();

        local_v<vec2_s> z = shader.local<vec2_s>();
        z.store(coord);

        local_v<int_s> num_its = shader.local<int_s>();
        num_its.store(mandelbrot_iterations);

        int_v i = shader.forLoop(mandelbrot_iterations);
        {
            vec2_v zl = z.load();
            float_v a = zl[0];
            float_v b = zl[1];

            float_v r = a * a + b * b;

            shader.ifThen(r > max_rad);
            {
                num_its.store(i);
                shader.breakLoop();
            }
            shader.endIf();

            vec2_v new_z = vec2_s::cons(a * a - b * b, 2.f * a * b) + coord;
            z.store(new_z);
        }
        shader.endLoop();

        float_v itnum = cast<float_s>(num_its.load());

        float_v r = (sin(itnum * 0.143f) + 1.0f) / 2.0f;
        float_v g = (cos(itnum * 0.273f) + 1.0f) / 2.0f;
        float_v b = (sin(itnum * 0.352f) + 1.0f) / 2.0f;

        vec4_v black = vec4_s::cons(0.0f, 0.0f, 0.0f, 1.0f);

        vec4_v out_col = select(itnum < mandelbrot_iterations,
                                vec4_s::cons(r, g, b, 1.0f), black);

        shader.compile(fragment_spirv, out_col);

        // SUtils::binaryPrettyPrint(fragment_spirv);
    }

    /* for(int i = 0; i < fragment_spirv.size(); i++) {
       printf("%d\n", fragment_spirv[i]);
       } */

    wg::ShaderPtr fragment_shader = wing.createShader(wg::ShaderStage::Fragment, fragment_spirv);
    wg::PipelinePtr pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                       {vertex_shader, fragment_shader});

    wg::BasicDrawPassSettings draw_pass_settings;
    draw_pass_settings.render_pass_settings.setShouldClear(true);
    wg::DrawPassPtr draw_pass = wing.createBasicDrawPass(pipeline, draw_pass_settings);

    draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    draw_pass->getCommandChain().recordBindResourceSet(time_set, 0);
    draw_pass->getCommandChain().recordDraw(model.getVertexBuffers(), model.getIndexBuffer());
    draw_pass->getCommandChain().endRecording();

    draw_pass->getSemaphores().setWaitSemaphores({ wing.createAndAddImageReadySemaphore() });
    wing.setPresentWaitForSemaphores({ draw_pass->getSemaphores().createOnFinishSemaphore() });

    float f = 0.0;
    float inc = 0.01f;

    while (win.isOpen()) {
        /* if(f >= 0.01f || f <= 0.01f) {
            inc *= -1.0f;
        }
        f += inc; */
        f += inc;

        draw_pass->awaitCurrentCommand();

        time_uniform->setCurrent(sin(f));

        draw_pass->render();

        wing.present();

        win.flushEvents();
        if(win.isKeyPressed(WK_ESC)) {
            break;
        }
    }

    wing.waitIdle();
}
