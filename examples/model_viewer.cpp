#include <Winval.hpp>

#include <Wingine.hpp>
#include <WgUtils.hpp>

#include <spurv.hpp>

int main() {
  const int width = 800, height = 800;
  Winval win(width, height);
  wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());
  
  wgut::Model model = wgut::Model::fromFile(wing, "../models/teapot.obj",
					    {wgut::ReadAttribType::attTypePosition,
					    wgut::ReadAttribType::attTypeNormal}); 

  wg::Uniform cameraUniform = wing.createUniform<falg::Mat4>();

  std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  
  wg::ResourceSet resourceSet = wing.createResourceSet(resourceSetLayout);
  resourceSet.set({&cameraUniform});

  std::vector<wg::VertexAttribDesc> vertAttrDesc =
    std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
					0, // Binding no.
					3, // Number of elements
					3 * sizeof(float), // Stride (in bytes)
					0}, // Offset (bytes)
				       {wg::tFloat32, 1, 3, 3 * sizeof(float), 0}};

  std::vector<uint32_t> vertex_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_VERTEX, vec3_s, vec3_s> shader;
    vec3_v s_pos = shader.input<0>();
    vec3_v s_col = shader.input<1>();

    SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
    mat4_v trans = trans_bind.member<0>();

    // vec3_v ss_pos = (1.0f / 50.0f) * s_pos;
    vec4_v het = vec4_s::cons(s_pos[0], s_pos[1], s_pos[2], 1.0);
    vec4_v transformed_pos = trans * het;
    // vec4_v transformed_pos = (1.0f / 50.0f) * het - vec4_s::cons(0.0f, 0.0f, -1.0f, 0.0f);

    vec4_v hcol = vec4_s::cons(s_col[0], s_col[1], s_col[2], 1.0);
    // vec4_v hcol = vec4_s::cons(1.0f, 0.0f, 0.0f, 1.0f);
    
    shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
    shader.compile(vertex_spirv, hcol);
  }

  wg::Shader vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT, vec4_s> shader;
    vec4_v in_col = shader.input<0>();

    shader.compile(fragment_spirv, in_col);
  }

  wg::Shader fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);
  
  wg::Pipeline pipeline = wing.
    createPipeline(vertAttrDesc,
		   {&resourceSetLayout},
		   {&vertex_shader, &fragment_shader});

  wg::RenderFamily family = wing.createRenderFamily(pipeline, true);
  
  wgut::Camera camera(F_PI / 3.f, 9.0 / 8.0, 0.01f, 1000.0f);
  float phi = 0.0;

  while (win.isOpen()) {

    phi += 0.01;
    
    camera.setLookAt(3.0f * falg::Vec3(sin(phi), 1.0f, cos(phi)),
		     falg::Vec3(0.0f, 0.0f, 0.0f),
		     falg::Vec3(0.0f, 1.0f, 0.0f));
    
    falg::Mat4 renderMatrix = camera.getRenderMatrix();
    
    cameraUniform.set(renderMatrix);
    
    family.startRecording();
    family.recordDraw(model.getVertexBuffers(), model.getIndexBuffer(), {resourceSet});
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

  model.destroy(wing);
  wing.destroy(cameraUniform);

}
