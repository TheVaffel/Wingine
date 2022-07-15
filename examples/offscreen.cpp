
#include "../include/Wingine.hpp"
#include "../include/WgUtils.hpp"

#include <HGraf/improc_io.hpp>

#include <spurv.hpp>

#include "./renderdoc_util.hpp"

int main() {

    renderdoc::API* rdapi = renderdoc::init();

    const int width = 480, height = 360;

    wg::Wingine wing(width, height, "Offscreen");

    const int num_points = 3;
    const int num_triangles = 1;

    float positions[num_points * 4] = {
        1.0f, -1.0f, -2.5f, 1.0f,
        -1.0f, -1.0f, -2.5f, 1.0f,
        0.0f, 1.0f, -2.5f, 1.0f,
    };

    float colors[num_points * 4] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
    };

    uint32_t indices[num_triangles * 3] = {
        0, 1, 2,
    };

    wg::VertexBufferPtr<float> position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, 0, num_points * 4);

    wg::VertexBufferPtr<float> color_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    color_buffer->set(colors, 0, num_points * 4);

    wg::IndexBufferPtr index_buffer = wing.createIndexBuffer(num_triangles * 3);
    index_buffer->set(indices, 0, num_triangles * 3);

    wgut::Model model({position_buffer, color_buffer}, index_buffer);

    wg::UniformChainPtr<falg::Mat4> cameraUniform = wing.createUniformChain<falg::Mat4>();

    std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};

    wg::ResourceSetChainPtr resourceSetChain = wing.createResourceSetChain(resourceSetLayout);
    resourceSetChain->set({cameraUniform});

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {
        wg::VertexAttribDesc(0, wg::ComponentType::tFloat32, 4, 4 * sizeof(float), 0),
        wg::VertexAttribDesc(1, wg::ComponentType::tFloat32, 4, 4 * sizeof(float), 0)
    };

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec4_s, vec4_s> shader;
        vec4_v s_pos = shader.input<0>();
        vec4_v s_col = shader.input<1>();

        mat4_v trans = shader.uniformBinding<mat4_s>(0, 0).member<0>().load();

        vec4_v transformed_pos = trans * s_pos;

        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(vertex_spirv, s_col);

    }

    wg::ShaderPtr vertex_shader = wing.createShader(wg::ShaderStage::Vertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_FRAGMENT, vec4_s> shader;
        vec4_v in_col = shader.input<0>();

        shader.compile(fragment_spirv, in_col);
    }

    wg::ShaderPtr fragment_shader = wing.createShader(wg::ShaderStage::Fragment, fragment_spirv);

    wg::PipelinePtr pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                       {resourceSetLayout},
                       {vertex_shader, fragment_shader});

    wgut::Camera camera(F_PI / 3.f, 9.0 / 8.0, 0.01f, 100.0f);

    wg::BasicDrawPassSettings draw_pass_settings;
    draw_pass_settings.render_pass_settings.setShouldClear(true);
    wg::DrawPassPtr draw_pass = wing.createBasicDrawPass(pipeline, draw_pass_settings);

    draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    draw_pass->getCommandChain().recordDraw(model.getVertexBuffers(), model.getIndexBuffer(), {resourceSetChain});
    draw_pass->getCommandChain().endRecording();

    draw_pass->getSemaphores().setWaitSemaphores({ wing.createAndAddImageReadySemaphore() });

    wg::Semaphore semaphore0 = draw_pass->getSemaphores().createOnFinishSemaphore();
    wing.setPresentWaitForSemaphores({semaphore0});

    uint32_t stride = wing.getRenderedImageRowByteStride();
    hg::Image<std::array<unsigned char, 4>> image(width, height, stride);

    const uint32_t num_images = 3;

    for (uint32_t i = 0; i < num_images; i++) {
        renderdoc::start_capture(rdapi);

        falg::Mat4 renderMatrix = camera.getRenderMatrix();

        cameraUniform->setCurrentUniform(renderMatrix);


        draw_pass->render();

        wing.present();
        wing.copyLastRenderedImage((uint32_t*)image.getData());

        std::ostringstream file_name_oss;
        file_name_oss << "frame_" << i << ".png";

        hg::writeImage(image, file_name_oss.str());
        std::cout << "Wrote file " << file_name_oss.str() << std::endl;

        cameraUniform->swap();
        resourceSetChain->swap();

        renderdoc::end_capture(rdapi);

    }

    wing.waitIdle();
}
