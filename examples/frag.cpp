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

  
  float scale = 0.001f;
  float offx = -0.77568377f;
  float offy = 0.13646737f;

  std::vector<uint32_t> vertex_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_VERTEX, vec4_s> shader;
    vec4_v s_pos = shader.input<0>();
    uint_v vi = shader.getBuiltin<BUILTIN_VERTEX_INDEX>();
    // SUniformBinding<float_s> un1 = shader.uniformBinding<float_s>(0, 0);

    // Compute corners, (-1, -1) to (1, 1)
    float_v pv0 = cast<float_s>(vi % 2) * 2.f - 1.f;
    float_v pv1 = cast<float_s>(vi / 2) * 2.f - 1.f;

    vec2_v coord = vec2_s::cons(pv0, pv1) * scale + vec2_s::cons(offx, offy);

    shader.setBuiltin<BUILTIN_POSITION>(s_pos);
    shader.compile(vertex_spirv, coord);

  }

  wg::Shader* vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  int mandelbrot_iterations = 1000;
  float max_rad = 4.f;
  
  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT, vec2_s> shader;
   
    vec2_v coord = shader.input<0>();

    vec2_lv z = shader.local<vec2_s>();
    z.store(coord);

    int_lv num_its = shader.local<int_s>();
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

    SUtils::binaryPrettyPrint(fragment_spirv);
  }

  /* for(int i = 0; i < fragment_spirv.size(); i++) {
    printf("%d\n", fragment_spirv[i]);
    } */

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
