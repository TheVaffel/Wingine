#include <Winval.hpp>

#include <Wingine.hpp>
#include <WgUtils.hpp>

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
    unsigned char* texture_buffer = new unsigned char[texture_width * texture_height * 4];
    for(int i = 0; i < texture_height; i++) {
        for(int j = 0; j < texture_width; j++) {
            for(int k = 0; k < 4; k++) {
                texture_buffer[(i * texture_width + j) * 4 + k] = (((i >> 5) ^ (j >> 5)) & 1) ? 0x00 : 0xff;
            }
        }
    }
  
    wg::ResourceImage* tex_im = wing.createResourceImage(texture_width, texture_height);

    std::vector<uint64_t> computeSetLayout = { wg::resImage | wg::shaCompute };

    wg::ResourceSet* compute_set = wing.createResourceSet(computeSetLayout);
    compute_set->set({tex_im});

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
  
    wg::Shader* compute_shader = wing.createShader(wg::shaCompute, compute_spirv);

    wg::ComputePipeline* compute_pipeline = wing.createComputePipeline({computeSetLayout},
                                                                       {compute_shader});

    wg::SemaphoreChain* chain = wing.createSemaphoreChain();
  
    wing.dispatchCompute(compute_pipeline,
                         {compute_set},
                         {chain}, texture_width, texture_height);

    wg::Texture* texture = wing.createTexture(texture_width, texture_height);
    // texture->set(texture_buffer, {chain});
    texture->set(tex_im, {chain});

    delete[] texture_buffer;

    wg::VertexBuffer<float>* position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, num_points * 4);
  
    wg::VertexBuffer<float>* tex_coord_buffer =
        wing.createVertexBuffer<float>(num_points * 2);
    tex_coord_buffer->set(tex_coords, num_points * 2);

    wg::IndexBuffer* index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
    index_buffer->set(indices, num_triangles * 3);


    std::vector<uint64_t> resourceSetLayout = {wg::resTexture | wg::shaFragment};
  
    wg::ResourceSet* resourceSet = wing.createResourceSet(resourceSetLayout);
    resourceSet->set({texture});
  
    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
                                            0, // Binding no.
                                            4, // Number of elements
                                            4 * sizeof(float), // Stride (in bytes)
                                            0}, // Offset (bytes)
                                           {wg::tFloat32, 1, 2, 2 * sizeof(float), 0}};

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        VertexShader<vec4_s, vec2_s> shader;
        vec4_v s_pos = shader.input<0>();
        vec2_v s_coord = shader.input<1>();
    
        shader.setBuiltin<BUILTIN_POSITION>(s_pos);
        shader.compile(vertex_spirv, s_coord);
    }

    wg::Shader* vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        FragmentShader<vec2_s> shader;
        vec2_v coord = shader.input<0>();

        texture2D_v texture = shader.uniformConstant<texture2D_s>(0, 0).load();

        vec4_v color = texture[coord];
    
        shader.compile(fragment_spirv, color);
    }

    wg::Shader* fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
    wg::Pipeline* pipeline = wing.
        createPipeline(vertAttrDesc,
                       {resourceSetLayout},
                       {vertex_shader, fragment_shader});

    wg::RenderFamily* family = wing.createRenderFamily(pipeline, true);
    
    family->startRecording();
    family->recordDraw({position_buffer, tex_coord_buffer}, index_buffer, {resourceSet});
    family->endRecording();
  
    while (win.isOpen()) {

        family->submit({chain});
    
        wing.present({chain});

        win.sleepMilliseconds(40);

        wing.waitForLastPresent();

        win.flushEvents();
        if(win.isKeyPressed(WK_ESC)) {
            break;
        }
    }

    wing.destroy(chain);

    wing.destroy(tex_im);
    wing.destroy(compute_set);
    wing.destroy(compute_shader);
    wing.destroy(compute_pipeline);
  
    wing.destroy(family);

    wing.destroy(vertex_shader);
    wing.destroy(fragment_shader);
  
    wing.destroy(pipeline);

    wing.destroy(position_buffer);
    wing.destroy(tex_coord_buffer);
    wing.destroy(index_buffer);

    wing.destroy(texture);

}
