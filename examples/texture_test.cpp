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

  wg::Texture texture = wing.createTexture(texture_width, texture_height);
  texture->set(texture_buffer);

  wg::VertexBuffer<float> position_buffer =
    wing.createVertexBuffer<float>(num_points * 4);
  position_buffer.set(positions, num_points * 4);
  
  wg::VertexBuffer<float> tex_coord_buffer =
    wing.createVertexBuffer<float>(num_points * 2);
  tex_coord_buffer.set(tex_coords, num_points * 2);

  wg::IndexBuffer index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
  index_buffer.set(indices, num_triangles * 3);

  wg::RenderObject triangle({&position_buffer, &tex_coord_buffer}, index_buffer);


  std::vector<uint64_t> resourceSetLayout = {wg::resTexture | wg::shaFragment};
  
  wg::ResourceSet resourceSet = wing.createResourceSet(resourceSetLayout);
  resourceSet.set({texture});
  
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

    SShader<SShaderType::SHADER_VERTEX, vec4_s, vec2_s> shader;
    vec4_v s_pos = shader.input<0>();
    vec2_v s_coord = shader.input<1>();
    
    shader.setBuiltin<BUILTIN_POSITION>(s_pos);
    shader.compile(vertex_spirv, s_coord);
  }

  wg::Shader vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT, vec2_s> shader;
    vec2_v coord = shader.input<0>();

    texture2D_v texture = shader.uniformBinding<texture2D_s>(0, 0);

    vec4_v color = texture[coord];
    
    shader.compile(fragment_spirv, color);
  }

  wg::Shader fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
  wg::Pipeline pipeline = wing.
    createPipeline(vertAttrDesc,
		   {&resourceSetLayout},
		   {&vertex_shader, &fragment_shader});

  wg::RenderFamily family = wing.createRenderFamily(pipeline, true);

  while (win.isOpen()) {
    
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
  wing.destroy(tex_coord_buffer);
  wing.destroy(index_buffer);

  wing.destroy(texture);

}
