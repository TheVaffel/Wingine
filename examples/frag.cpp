#include <Winval.hpp>

#include <Wingine.hpp>
#include <WgUtils.hpp>

#include <spurv.hpp>

int main() {
  const int width = 800, height = 800;
  Winval win(width, height);
  wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());
  
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

  wg::VertexBuffer<float> position_buffer =
    wing.createVertexBuffer<float>(num_points * 4);
  position_buffer.set(positions, num_points * 4);

  wg::IndexBuffer index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
  index_buffer.set(indices, num_triangles * 3);

  wgut::Model model({&position_buffer}, index_buffer);

  std::vector<wg::VertexAttribDesc> vertAttrDesc =
    std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
					0, // Binding no.
					4, // Number of elements
					4 * sizeof(float), // Stride (in bytes)
					0}, // Offset (bytes)
  };

  std::vector<uint32_t> vertex_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_VERTEX, vec4_s> shader;
    vec4_v s_pos = shader.input<0>();

    shader.setBuiltin<BUILTIN_POSITION>(s_pos);
    shader.compile(vertex_spirv);
  }

  wg::Shader vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT> shader;
    
    vec4_v frag_coord = shader.getBuiltin<BUILTIN_FRAG_COORD, vec4_s>();
    vec4_v out_col = vec4_s::cons(frag_coord[0] / float(width),
				  frag_coord[1] / float(height), 0.50f, 1.0f);
    
    shader.compile(fragment_spirv, out_col);
  }

  wg::Shader fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
  wg::Pipeline pipeline = wing.
    createPipeline(vertAttrDesc,
		   {},
		   {&vertex_shader, &fragment_shader});

  wg::RenderFamily family = wing.createRenderFamily(pipeline, true);
  

  while (win.isOpen()) {
    
    family.startRecording();
    family.recordDraw(model.getVertexBuffers(), model.getIndexBuffer(), {});
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
  
  wing.destroy(pipeline);

  wing.destroy(position_buffer);
  wing.destroy(index_buffer);

}
