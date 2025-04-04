#include <Winval.hpp>

#include "../include/Wingine.hpp"
#include "../include/WgUtils.hpp"

#include <spurv.hpp>

#include <generelle/modelling.hpp>

namespace gn = generelle;

int main() {
    const int width = 800, height = 800;
    Winval win(width, height);
    wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());

    gn::GE scene = gn::makeSphere(1.0f)
	.translate(falg::Vec3(0.01f, 0.01f, 0.01f))
	.scale(falg::Vec3(1.0f, 0.5f, 1.0f))
        .add(gn::makeBox(falg::Vec3(0.5f, 0.5f, 1.0f))
	     .translate(falg::Vec3(0.0f, 0.0f, 1.0f)));

    gn::GE scene2 = gn::makeSphere(1.0f)
	.translate(falg::Vec3(-0.5f, 0.0f, 0.0f))
	.add(gn::makeSphere(1.0f)
	     .translate(falg::Vec3(0.5f, 0.0f, 0.0f)))
	.add(gn::makeCylinder(1.0f, 1.0f).translate(falg::Vec3(0.0f, 1.0f, 0.0f)));

    gn::MeshConstructor::ConstructMeshSetup meshSetup1, meshSetup2;

    meshSetup1.numRectify = 2;
    meshSetup2.numRectify = 3;

    gn::Mesh mesh = gn::MeshConstructor::constructMesh(scene, 0.2f, 10.0f, falg::Vec3(1.0f, 1.0f, 1.0f), meshSetup1);
    gn::Mesh mesh2 = gn::MeshConstructor::constructMesh(scene, 0.2f, 10.0f, falg::Vec3(1.0f, 1.0f, 1.0f), meshSetup2);

    std::vector<float> positions, normals;
    for (unsigned int i = 0; i < mesh.positions.size(); i++) {
        for (int j = 0; j < 3; j++) {
            positions.push_back(mesh.positions[i][j]);
            normals.push_back(mesh.normals[i][j]);
        }
    }

    std::vector<float> positions2 = std::vector<float>((float*)mesh2.positions.data(),
                                                       (float*)(mesh2.positions.data() + mesh2.positions.size()));
    std::vector<float> normals2 = std::vector<float>((float*)mesh2.normals.data(),
                                                     (float*)(mesh2.normals.data() + mesh2.normals.size()));

    wgut::Model model = wgut::Model::constructModel(wing, {positions, normals}, mesh.indices);
    wgut::Model model2 = wgut::Model::constructModel(wing, {positions2, normals2}, mesh2.indices);


    wg::UniformChainPtr<falg::Mat4> cameraUniform = wing.createUniformChain<falg::Mat4>();

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {
        wg::VertexAttribDesc(0, wg::ComponentType::Float32, 3, 3 * sizeof(float), 0), // Offset (bytes)
        wg::VertexAttribDesc(1, wg::ComponentType::Float32, 3, 3 * sizeof(float), 0)
    };

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec3_s, vec3_s> shader;
        vec3_v s_pos = shader.input<0>();
        vec3_v s_col = shader.input<1>();

        auto trans_bind = shader.uniformBinding<mat4_s>(0, 0);
        mat4_v trans = trans_bind.member<0>().load();

        vec4_v het = vec4_s::cons(s_pos[0], s_pos[1], s_pos[2], 1.0);
        vec4_v transformed_pos = trans * het;

        vec4_v hcol = vec4_s::cons(s_col[0], s_col[1], s_col[2], 1.0);

        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(vertex_spirv, hcol);
    }

    wg::ShaderPtr vertex_shader = wing.createShader(wg::ShaderStage::Vertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_FRAGMENT, vec4_s> shader;
        vec4_v in_col = shader.input<0>();
        vec4_v final_col = (in_col + vec4_s::cons(1.0f, 1.0f, 1.0f, 1.0f)) * 0.5;

        shader.compile(fragment_spirv, final_col);
    }

    std::vector<uint32_t> black_fragment_spirv;
    {
        using namespace spurv;
        FragmentShader<vec4_s> shader;
        shader.input<0>(); // Avoids runtime problem, since we don't use input here
        vec4_v in_col = vec4_s::cons(0.0f, 0.0f, 0.0f, 1.0f);
        shader.compile(black_fragment_spirv, in_col);
    }

    wg::ShaderPtr fragment_shader = wing.createShader(wg::ShaderStage::Fragment, fragment_spirv);
    wg::ShaderPtr black_fragment_shader = wing.createShader(wg::ShaderStage::Fragment, black_fragment_spirv);

    wg::BasicPipelineSetup pipelineSetup;
    pipelineSetup.setPolygonMode(wg::PolygonMode::Fill);
    wg::PipelinePtr pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                       {vertex_shader, fragment_shader}, pipelineSetup);

    pipelineSetup.setPolygonMode(wg::PolygonMode::Line);
    wg::PipelinePtr line_pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                       {vertex_shader, black_fragment_shader}, pipelineSetup);

    wg::BasicDrawPassSettings draw_settings;
    draw_settings.render_pass_settings.setShouldClear(true);
    wg::DrawPassPtr polygon_draw_pass = wing.createBasicDrawPass(pipeline, draw_settings);

    draw_settings.render_pass_settings.setShouldClear(false);
    wg::DrawPassPtr line_draw_pass = wing.createBasicDrawPass(line_pipeline, draw_settings);

    wgut::Camera camera(F_PI / 3.f, 9.0 / 8.0, 0.01f, 1000.0f);

    polygon_draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    polygon_draw_pass->getCommandChain().recordBindResourceSet(0, {{ 0, cameraUniform }});
    polygon_draw_pass->getCommandChain().recordDraw(model.getVertexBuffers(), model.getIndexBuffer());
    polygon_draw_pass->getCommandChain().endRecording();

    line_draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    line_draw_pass->getCommandChain().recordBindResourceSet(0, {{ 0, cameraUniform }});
    line_draw_pass->getCommandChain().recordDraw(model.getVertexBuffers(), model.getIndexBuffer());
    line_draw_pass->getCommandChain().endRecording();

    int lock_x = width / 2, lock_y = height / 2;

    win.lockPointer(true, width / 2, height / 2);
    win.setPointerVisible(false);

    float pointer_speed = 0.005f;
    float zoom_speed = 0.2f;

    float radius = 3.0f;

    float phi = 0.0, theta = 0.0;
    int diff_x = 0, diff_y = 0, scroll = 0;
    bool is_switch = false;
    int switch_state = 0;

    wing.setPresentWaitForSemaphores({
            polygon_draw_pass->getSemaphores().createOnFinishSemaphore(),
            line_draw_pass->getSemaphores().createOnFinishSemaphore()
        });

    while (win.isOpen()) {

        phi = phi + diff_x * pointer_speed;
        theta = std::min(F_PI / 2 - 0.02f, std::max(- F_PI / 2 + 0.02f, theta - diff_y * pointer_speed));
        radius += zoom_speed * scroll;

        camera.setLookAt(radius * falg::Vec3(sin(phi) * cos(theta), sin(theta), cos(phi) * cos(theta)),
                         falg::Vec3(0.0f, 0.0f, 0.0f),
                         falg::Vec3(0.0f, 1.0f, 0.0f));

        falg::Mat4 renderMatrix = camera.getRenderMatrix();

        polygon_draw_pass->awaitCurrentCommand();
        line_draw_pass->awaitCurrentCommand();

        cameraUniform->setCurrent(renderMatrix);

        polygon_draw_pass->render();
        line_draw_pass->render();

        wing.present();

        win.flushEvents();
        win.getPointerPosition(&diff_x, &diff_y);

        diff_x -= lock_x;
        diff_y -= lock_y;
        scroll = win.getScroll();
        bool new_is_switch = win.isKeyPressed(WK_B);
        if (new_is_switch && !is_switch) {
            wing.waitIdle();

            switch_state = switch_state ^ 1;

            wgut::Model& curr_model = switch_state == 0 ? model : model2;

            polygon_draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
            polygon_draw_pass->getCommandChain().recordBindResourceSet(0, {{ 0, cameraUniform }});
            polygon_draw_pass->getCommandChain().recordDraw(curr_model.getVertexBuffers(), curr_model.getIndexBuffer());
            polygon_draw_pass->getCommandChain().endRecording();

            line_draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
            line_draw_pass->getCommandChain().recordBindResourceSet(0, {{ 0, cameraUniform }});
            line_draw_pass->getCommandChain().recordDraw(curr_model.getVertexBuffers(), curr_model.getIndexBuffer());
            line_draw_pass->getCommandChain().endRecording();
        }

        is_switch = new_is_switch;

        if(win.isKeyPressed(WK_ESC)) {
            break;
        }
    }

    wing.waitIdle();
}
