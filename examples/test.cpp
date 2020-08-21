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
        -1.0f, -1.0f, -2.5f, 1.0f,
        1.0f, -1.0f, -2.5f, 1.0f,
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

    wg::VertexBuffer<float>* position_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    position_buffer->set(positions, num_points * 4);
  
    wg::VertexBuffer<float>* color_buffer =
        wing.createVertexBuffer<float>(num_points * 4);
    color_buffer->set(colors, num_points * 4);

    wg::IndexBuffer* index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
    index_buffer->set(indices, num_triangles * 3);

    wgut::Model model({position_buffer, color_buffer}, index_buffer);

    wg::Uniform<falg::Mat4>* cameraUniform = wing.createUniform<falg::Mat4>();

    std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  
    wg::ResourceSet* resourceSet = wing.createResourceSet(resourceSetLayout);
    resourceSet->set({cameraUniform});

    std::vector<wg::VertexAttribDesc> vertAttrDesc =
        std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
                                            0, // Binding no.
                                            4, // Number of elements
                                            4 * sizeof(float), // Stride (in bytes)
                                            0}, // Offset (bytes)
                                           {wg::tFloat32, 1, 4, 4 * sizeof(float), 0}};

    std::vector<uint32_t> vertex_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_VERTEX, vec4_s, vec4_s> shader;
        vec4_v s_pos = shader.input<0>();
        vec4_v s_col = shader.input<1>();

        // SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
        mat4_v trans = shader.uniformBinding<mat4_s>(0, 0).member<0>().load();

        vec4_v transformed_pos = trans * s_pos;
    
        shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
        shader.compile(vertex_spirv, s_col);

        // prettyprint(vertex_spirv);
    }

    /* std::cout << "Vertex SPIRV:" << std::endl;
       for(uint i = 0; i < vertex_spirv.size(); i++) {
       std::cout << vertex_spirv[i] << std::endl;
       } */

    wg::Shader* vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

    std::vector<uint32_t> fragment_spirv;
    {
        using namespace spurv;

        SShader<SShaderType::SHADER_FRAGMENT, vec4_s> shader;
        vec4_v in_col = shader.input<0>();

        shader.compile(fragment_spirv, in_col);
    }

    /* std::cout << "Fragment SPIRV:" << std::endl;
       for(uint i = 0; i < fragment_spirv.size(); i++) {
       std::cout << fragment_spirv[i] << std::endl;
       } */

    wg::Shader* fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
    wg::Pipeline* pipeline = wing.
        createPipeline(vertAttrDesc,
                       {resourceSetLayout},
                       {vertex_shader, fragment_shader});

    wg::RenderFamily* family = wing.createRenderFamily(pipeline, true);
  
    wgut::Camera camera(F_PI / 3.f, 9.0 / 8.0, 0.01f, 100.0f);
  
    family->startRecording();
    family->recordDraw(model.getVertexBuffers(), model.getIndexBuffer(), {resourceSet});
    family->endRecording();

    wg::SemaphoreChain* chain = wing.createSemaphoreChain();
  
    while (win.isOpen()) {
        falg::Mat4 renderMatrix = camera.getRenderMatrix();
    
        cameraUniform->set(renderMatrix);

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

    wing.destroy(family);

    wing.destroy(vertex_shader);
    wing.destroy(fragment_shader);
  
    wing.destroy(pipeline);

    wing.destroy(position_buffer);
    wing.destroy(color_buffer);
    wing.destroy(index_buffer);

    wing.destroy(cameraUniform);

}
