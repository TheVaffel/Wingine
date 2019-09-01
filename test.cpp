#include <Winval.hpp>

#include <Wingine.hpp>

#define WG_UTILS_MATRIX_LIB_FLATALG
#include <WgUtils.hpp>

#include <spurv.hpp>

int main() {
  const int width = 800, height = 800;
  Winval win(width, height);
  wg::Wingine wing(width, height, win.getWindow(), win.getDisplay());

  // Hypothetical execution

  const int num_points = 3;
  const int num_triangles = 1;
  
  float positions[num_points * 4] = {
    -1.0f, -1.0f, 0.5f, 1.0f,
    1.0f, -1.0f, 0.5f, 1.0f,
    0.0f, 1.0f, 0.5f, 1.0f,

    /*
      -1.0f, -1.0f, -0.5f, 1.0f,
      1.0f, -1.0f, -0.5f, 1.0f,
      0.0f, 1.0f, -0.5f, 1.0f */
  };

  float colors[num_points * 4] = {
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,

    /*
      1.0f, 0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f, 1.0f */
  };

  uint32_t indices[num_triangles * 3] = {
    0, 1, 2,
    /* 0, 2, 1,

       3, 4, 5,
       3, 5, 4*/
  };

  wg::VertexBuffer<float> position_buffer =
    wing.createVertexBuffer<float>(num_points * 4);
  position_buffer.set(positions, num_points * 4);
  
  wg::VertexBuffer<float> color_buffer =
    wing.createVertexBuffer<float>(num_points * 4);
  color_buffer.set(colors, num_points * 4);

  wg::IndexBuffer index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
  index_buffer.set(indices, num_triangles * 3);

  wg::RenderObject triangle({&position_buffer, &color_buffer}, index_buffer);

  //   wg::Uniform cameraUniform = wing.createUniform<Matrix4>();

  wg::Uniform floatUniform = wing.createUniform<float>();
  
  std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  
  wg::ResourceSet resourceSet = wing.createResourceSet(resourceSetLayout);
  // resourceSet.set({&cameraUniform});
  resourceSet.set({&floatUniform});

  std::vector<wg::VertexAttribDesc> vertAttrDesc =
    std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
					0, // Binding no.
					4, // Number of elements
					4 * sizeof(float), // Stride (in bytes)
					0},
				       {wg::tFloat32, 1, 4, 4 * sizeof(float), 0}}; // Offset (bytes)

  std::vector<uint32_t> vertex_spirv;
  {
    using namespace spurv;

    SShader<SHADER_VERTEX, vec4_s, vec4_s> shader;
    vec4_v s_pos = shader.input<0>();
    vec4_v s_col = shader.input<1>();

    SUniformBinding<float_s> trans_bind = shader.uniformBinding<float_s>(0, 0);
    float_v trans = trans_bind.member<0>();

    // vec4_v transformed_pos = trans * s_pos;
    
    // shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
    shader.setBuiltin<BUILTIN_POSITION>(s_pos);
    shader.compile(vertex_spirv, s_col * trans);
  }

  wg::Shader vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SHADER_FRAGMENT, vec4_s> shader;
    vec4_v in_col = shader.input<0>();

    shader.compile(fragment_spirv, in_col);
  }

  wg::Shader fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
  wg::Pipeline pipeline = wing.
    createPipeline(vertAttrDesc,
		   {&resourceSetLayout},
		   {&vertex_shader, &fragment_shader});

  wg::RenderFamily family = wing.createRenderFamily(pipeline, true);
  
  wgut::Camera camera(M_PI / 3, 9.0 / 16.0, 0.01f, 100.0f);

  float a = 0;
  while (win.isOpen()) {
    // Matrix4 renderMatrix = camera.getRenderMatrix();
    a += 0.03;
    if(a > 1.0) {
      a = 0;
    }
    // std::cout << renderMatrix.str() << std::endl;
    
    // cameraUniform.set(camera.getRenderMatrix());
    floatUniform.set(a);
    
    family.startRecording();
    family.recordDraw(triangle, {resourceSet});
    family.endRecording();

    wing.present();

    win.sleepMilliseconds(40);

    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)) {
      break;
    }
  }

  wing.destroy(family);

  wing.destroy(vertex_shader);
  wing.destroy(fragment_shader);
  
  wing.destroy(resourceSet);
  wing.destroy(pipeline);

  wing.destroy(position_buffer);
  wing.destroy(color_buffer);
  wing.destroy(index_buffer);

  wing.destroy(floatUniform);
  //   wing.destroy(cameraUniform);

}
