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

  wg::VertexBuffer<float>* position_buffer =
    wing.createVertexBuffer<float>(num_points * 4);
  position_buffer->set(positions, num_points * 4);

  wg::IndexBuffer* index_buffer = wing.createIndexBuffer(num_triangles * 3); // Num indices
  index_buffer->set(indices, num_triangles * 3);

  wgut::Model model({position_buffer}, index_buffer);

  wg::Uniform<float>* time_uniform = wing.createUniform<float>();
  std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  wg::ResourceSet* time_set = wing.createResourceSet(resourceSetLayout);
  time_set->set({time_uniform});

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
    uint_v vi = shader.getBuiltin<BUILTIN_VERTEX_INDEX>();
    SUniformBinding<float_s> un1 = shader.uniformBinding<float_s>(0, 0);
    float_v oscil = un1.member<0>();

    float_v pv0 = cast<float_s>(vi % 2);
    float_v pv1 = cast<float_s>(vi / 2);

    float_lv local_variable = shader.local<float_s>();

    local_variable.store(float_s::cons(0.3f));

    float_v con = local_variable.load();
    
    local_variable.store(pv0);
    
    int_v i = shader.forLoop(4);
    {
      float_v a = local_variable.load();
      float_v b = a * 1.2f;
      local_variable.store(b);
      
    }
    shader.endLoop();

    
    float_v pv0p = local_variable.load();
    
    
    vec4_v col = vec4_s::cons(vec3_s::cons(pv0p, pv1, 0.3f) * oscil, 1.0f);

    shader.setBuiltin<BUILTIN_POSITION>(s_pos);
    shader.compile(vertex_spirv, col);
  }


  wg::Shader* vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT, vec4_s> shader;
    
    // vec4_v frag_coord = shader.getBuiltin<BUILTIN_FRAG_COORD>();
    vec4_v out_col = shader.input<0>();

    shader.compile(fragment_spirv, out_col);
  } 

  wg::Shader* fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
  wg::Pipeline* pipeline = wing.
    createPipeline(vertAttrDesc,
		   {resourceSetLayout},
		   {vertex_shader, fragment_shader});

  wg::RenderFamily* family = wing.createRenderFamily(pipeline, true);

  family->startRecording();
  family->recordDraw(model.getVertexBuffers(), model.getIndexBuffer(), {time_set});
  family->endRecording();
  
  float f = 0.0;
  float inc = 0.05f;
  while (win.isOpen()) {
    if(f > 1) {
      inc = -0.05f;
    } else if(f < 0) {
      inc = 0.05f;
    }
    f += inc;
    time_uniform->set(f);

    family->submit();
    
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

  wing.destroy(time_uniform);
}
