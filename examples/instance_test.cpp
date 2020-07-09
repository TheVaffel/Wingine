#include <Winval.hpp>

#include <Wingine.hpp>
#include <WgUtils.hpp>

#include <spurv.hpp>

#include <random>

int main() {
  int width = 800, height = 800;
  Winval win(width, height);
  wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());

  width = wing.getWindowWidth();
  height = wing.getWindowHeight();
  
  wgut::Model model = wgut::Model::fromFile(wing, "../models/teapot.obj",
					    {wgut::ReadAttribType::attTypePosition,
					    wgut::ReadAttribType::attTypeNormal});

  std::default_random_engine generator;
  std::uniform_real_distribution<float> dist(-50.0, 50.0);
  std::uniform_real_distribution<float> dist1(0.0, 1.0);
  
  const int num_instances = 20000;
  float offsets[3 * num_instances];
  float colors[3 * num_instances];
  for(int i = 0; i < num_instances; i++) {
    falg::Vec3 off;
    do {
      off = falg::Vec3(dist(generator), dist(generator), dist(generator));
    } while(off.sqNorm() <= 5.0f * 5.0f);

    for(int j = 0; j < 3; j++) {
      offsets[3 * i + j] = off[j];
      colors[3 * i + j] = dist1(generator);
    }
  }

  wg::VertexBuffer<float>* offset_buffer = wing.createVertexBuffer<float>(3 * num_instances);
  offset_buffer->set(offsets, 3 * num_instances);

  wg::VertexBuffer<float>* color_buffer = wing.createVertexBuffer<float>(3 * num_instances);
  color_buffer->set(colors, 3 * num_instances);

  wg::Uniform<falg::Mat4>* cameraUniform = wing.createUniform<falg::Mat4>();

  std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  
  wg::ResourceSet* resourceSet = wing.createResourceSet(resourceSetLayout);
  resourceSet->set({cameraUniform});

  // Positions, color, offset
  std::vector<wg::VertexAttribDesc> vertAttrDesc =
    std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
					0, // Binding no.
					3, // Number of elements
					3 * sizeof(float), // Stride (in bytes)
					0}, // Offset (bytes)
				       // Color
				       {wg::tFloat32, 1, 3, 3 * sizeof(float), 0, true}, // true - per_instance
				       // Offset
				       {wg::tFloat32, 2, 3, 3 * sizeof(float), 0, true}
  };

  std::vector<uint32_t> vertex_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_VERTEX, vec3_s, vec3_s, vec3_s> shader;
    vec3_v s_pos = shader.input<0>();
    vec3_v s_off = shader.input<1>();
    vec3_v s_col = shader.input<2>();

    SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
    mat4_v trans = trans_bind.member<0>();

    // vec3_v ss_pos = (1.0f / 50.0f) * s_pos;
    vec3_v offed = s_pos + s_off;
    
    vec4_v het = vec4_s::cons(offed[0], offed[1], offed[2], 1.0);
    vec4_v transformed_pos = trans * het;
    // vec4_v transformed_pos = (1.0f / 50.0f) * het - vec4_s::cons(0.0f, 0.0f, -1.0f, 0.0f);

    vec4_v hcol = vec4_s::cons(s_col[0], s_col[1], s_col[2], 1.0);
    // vec4_v hcol = vec4_s::cons(1.0f, 0.0f, 0.0f, 1.0f);
    
    shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
    shader.compile(vertex_spirv, hcol);
  }

  wg::Shader* vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT, vec4_s> shader;
    vec4_v in_col = shader.input<0>();

    shader.compile(fragment_spirv, in_col);
  }

  wg::Shader* fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
  wg::Pipeline* pipeline = wing.
    createPipeline(vertAttrDesc,
		   {resourceSetLayout},
		   {vertex_shader, fragment_shader});

  wg::RenderFamily* family = wing.createRenderFamily(pipeline, true);
  
  wgut::Camera camera(F_PI / 3.f, (float)height / (float)width, 0.01f, 1000.0f);
  float phi = 0.0;
      
  family->startRecording();
  family->recordDraw({model.getVertexBuffers()[0], offset_buffer, color_buffer},
		     model.getIndexBuffer(), {resourceSet}, num_instances);
  family->endRecording();

  
  while (win.isOpen()) {

    phi += 0.01;
    
    camera.setLookAt(3.0f * falg::Vec3(sin(phi), 1.0f, cos(phi)),
		     falg::Vec3(0.0f, 0.0f, 0.0f),
		     falg::Vec3(0.0f, 1.0f, 0.0f));
    
    falg::Mat4 renderMatrix = camera.getRenderMatrix();
    
    cameraUniform->set(renderMatrix);

    family->submit();
    
    wing.present();

    win.sleepMilliseconds(16);

    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)) {
      break;
    }
  }

  wing.destroy(family);

  wing.destroy(color_buffer);
  wing.destroy(offset_buffer);

  wing.destroy(vertex_shader);
  wing.destroy(fragment_shader);
  
  wing.destroy(pipeline);

  model.destroy(wing);
  wing.destroy(cameraUniform);

}
