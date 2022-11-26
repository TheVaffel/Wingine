#include <Winval.hpp>

#include "../include/Wingine.hpp"
#include "../include/WgUtils.hpp"

#include <spurv.hpp>

int main() {
    const int width = 800, height = 800;
    Winval win(width, height);
    wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());

    const int num_points = 3;
    const int num_triangles = 1;

    float positions[num_points * 4] = {
        -1.0f, -1.0f, 0.5f, 1.0f,
        1.0f, -1.0f, 0.5f, 1.0f,
        0.0f, 1.0f, 0.5f, 1.0f,
    };

    float tex_coords[num_points * 2] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f
    };

    uint32_t indices[num_triangles * 3] = {
        0, 1, 2,
    };

    const int texture_width = 400, texture_height = 400;
    std::vector<uint32_t> texture_buffer = std::vector<uint32_t>(texture_width * texture_height * 4);
    unsigned char* texture_data = reinterpret_cast<unsigned char*>(texture_buffer.data());
    for(int i = 0; i < texture_height; i++) {
        for(int j = 0; j < texture_width; j++) {
            for(int k = 0; k < 4; k++) {
                texture_data[(i * texture_width + j) * 4 + k] = (((i >> 5) ^ (j >> 5)) & 1) ? 0x00 : 0xff;
            }
        }
    }

    std::vector<uint32_t> compute_spirv;
    {
        using namespace spurv;
        ComputeShader shader;

        uvec3_v global_id_1 = shader.getBuiltin<BUILTIN_GLOBAL_INVOCATION_ID>();

        uvec2_v global_id = uvec2_s::cons(global_id_1[0], global_id_1[1]);

        image2D_v out_image = shader.uniformConstant<image2D_s>(0, 0).load();

        vec2_v coord = cast<vec2_s>(global_id) / vec2_s::cons(texture_width, texture_height);

        out_image.store(global_id, vec4_s::cons(coord[0], 0.0f, coord[1], 1.0f));

        shader.compile(compute_spirv);
    }

    wg::ShaderPtr compute_shader = wing.createShader(wg::ShaderStage::Compute, compute_spirv);

    std::vector<uint64_t> computeSetLayout = { wg::resImage | wg::shaCompute };
    wg::ComputePipelinePtr compute_pipeline = wing.createComputePipeline({computeSetLayout},
                                                                         {compute_shader});

    wg::StorageTexturePtr storage_texture = wing.createStorageTexture(texture_width, texture_height);
    wg::ResourceSetChainPtr compute_set = wing.createResourceSetChain(computeSetLayout, storage_texture->getStorageImage());

    compute_pipeline->execute({ compute_set }, texture_width, texture_height);
    compute_pipeline->awaitExecution();

    storage_texture->makeIntoTextureSync();

    wg::SettableTexturePtr texture = wing.createSettableTexture(texture_width, texture_height);
    texture->set(texture_buffer, 0);

    wg::VertexBufferPtr<float> position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, 0, num_points * 4);

    wg::VertexBufferPtr<float> tex_coord_buffer =
        wing.createVertexBuffer<float>(num_points * 2);
    tex_coord_buffer->set(tex_coords, 0, num_points * 2);

    wg::IndexBufferPtr index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
    index_buffer->set(indices, 0, num_triangles * 3);


    std::vector<uint64_t> resourceSetLayout = {wg::resTexture | wg::shaFragment};

    wg::ResourceSetChainPtr resourceSet = wing.createResourceSetChain(resourceSetLayout, storage_texture->getTexture());

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {
        wg::VertexAttribDesc(0,
                             wg::ComponentType::Float32, // Component type
                             4, // Number of elements
                             4 * sizeof(float), // Stride (in bytes)
                             0), // Offset (bytes)
        wg::VertexAttribDesc(1, wg::ComponentType::Float32, 2, 2 * sizeof(float), 0)};

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        VertexShader<vec4_s, vec2_s> shader;
        vec4_v s_pos = shader.input<0>();
        vec2_v s_coord = shader.input<1>();

        shader.setBuiltin<BUILTIN_POSITION>(s_pos);
        shader.compile(vertex_spirv, s_coord);
    }

    wg::ShaderPtr vertex_shader = wing.createShader(wg::ShaderStage::Vertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        FragmentShader<vec2_s> shader;
        vec2_v coord = shader.input<0>();

        texture2D_v texture = shader.uniformConstant<texture2D_s>(0, 0).load();

        vec4_v color = texture[coord];

        shader.compile(fragment_spirv, color);
    }

    wg::ShaderPtr fragment_shader = wing.createShader(wg::ShaderStage::Fragment, fragment_spirv);

    wg::PipelinePtr pipeline = wing.
        createBasicPipeline(vertAttrDesc,
                       {resourceSetLayout},
                       {vertex_shader, fragment_shader});

    wg::BasicDrawPassSettings draw_pass_settings;
    draw_pass_settings.render_pass_settings.setShouldClear(true);
    wg::DrawPassPtr draw_pass = wing.createBasicDrawPass(pipeline, draw_pass_settings);

    draw_pass->getCommandChain().startRecording(wing.getDefaultFramebufferChain());
    draw_pass->getCommandChain().recordBindResourceSet(resourceSet, 0);
    draw_pass->getCommandChain().recordDraw({position_buffer, tex_coord_buffer}, index_buffer);
    draw_pass->getCommandChain().endRecording();

    draw_pass->getSemaphores().setWaitSemaphores({ wing.createAndAddImageReadySemaphore() });
    wing.setPresentWaitForSemaphores({ draw_pass->getSemaphores().createOnFinishSemaphore() });

    while (win.isOpen()) {

        draw_pass->awaitCurrentCommand();
        draw_pass->render();

        wing.present();

        win.flushEvents();
        if(win.isKeyPressed(WK_ESC)) {
            break;
        }
    }

    wing.waitIdle();
}
