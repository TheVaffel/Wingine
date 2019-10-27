#include <Winval.hpp>

#include <Wingine.hpp>

#define WG_UTILS_MATRIX_LIB_FLATALG
#include <WgUtils.hpp>

#include <spurv.hpp>

int main() {
  const int width = 800, height = 800;
  Winval win(width, height);
  wg::Wingine wing(width, height, win.getWindow(), win.getDisplay());
  
  const int num_points = 7;
  const int num_triangles = 3;
  
  float positions[num_points * 4] = {
    -1.0f, -1.0f, -2.5f, 1.0f,
    1.0f, -1.0f, -2.5f, 1.0f,
    0.0f, 1.0f, -2.5f, 1.0f,

    // Plane
    -2.f, -1.f, -4.5f, 1.0f,
    2.f, -1.f, -4.5f, 1.0f,
    -2.f, -1.f, -1.5f, 1.0f,
    2.f, -1.f, -1.5f, 1.0f
  };

  float colors[num_points * 4] = {
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,

    0.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f
  };

  uint32_t indices[num_triangles * 3] = {
    0, 1, 2,
    3, 4, 5,
    5, 4, 6
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

  wg::Uniform cameraUniform = wing.createUniform<Matrix4>();

  std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  
  wg::ResourceSet resourceSet = wing.createResourceSet(resourceSetLayout);
  resourceSet.set({&cameraUniform});

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

    SShader<SHADER_VERTEX, vec4_s, vec4_s> shader;
    vec4_v s_pos = shader.input<0>();
    vec4_v s_col = shader.input<1>();

    SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
    mat4_v trans = trans_bind.member<0>();

    vec4_v transformed_pos = trans * s_pos;
    
    shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
    shader.compile(vertex_spirv, s_col);
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


  
  // Some random size
  const uint32_t shadow_buffer_width = 4000,
    shadow_buffer_height = 4000;

  wg::Framebuffer depth_framebuffer = wing.createFramebuffer(shadow_buffer_width,
							     shadow_buffer_height, true, true);
  wg::Texture shadow_texture = wing.createTexture(shadow_buffer_width,
						  shadow_buffer_height, true);

  std::vector<uint32_t> depth_vertex_shader;
  {
    using namespace spurv;

    SShader<SHADER_VERTEX, vec4_s> shader;
    SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
    mat4_v trans = trans_bind.member<0>();

    vec4_v pos = shader.input<0>();
    
    vec4_v transformed_pos = trans * pos;

    shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
    shader.compile(depth_vertex_shader);
  }

  wg::Shader depth_shader = wing.createShader(wg::shaVertex, depth_vertex_shader);

  
  wg::Pipeline depth_pipeline = wing.createPipeline({vertAttrDesc[0]},
						    {&resourceSetLayout},
						    {&depth_shader}, true); 
						    
  
  /* for(uint32_t i : depth_vertex_shader) {
    std::cout << i << std::endl;
    } */ 
  
  wg::RenderFamily family = wing.createRenderFamily(pipeline, true);

  wg::RenderFamily depth_family = wing.createRenderFamily(depth_pipeline, true);
  
  wgut::Camera camera(M_PI / 3.f, 9.0 / 8.0, 0.01f, 100.0f);
  camera.setLookAt(Vector3(2.0f, 2.0f, 2.0f),
		   Vector3(0.0f, 0.0f, -2.5f),
		   Vector3(0.0f, 1.0f, 0.0f));

  wgut::Camera light_camera(M_PI / 2.f, 1.0f, 0.01f, 100.0f);
  camera.setLookAt(Vector3(-2.0f, 2.0f, 2.0f),
		   Vector3(0.0f, 0.0f, -2.5f),
		   Vector3(0.0f, 1.0f, 0.0f));
		   

  float a = 0;
  while (win.isOpen()) {
    Matrix4 renderMatrix = camera.getRenderMatrix();
    a += 0.03;
    if(a > 1.0) {
      a = 0;
    }

    cameraUniform.set(light_camera.getRenderMatrix());

    depth_family.startRecording(depth_framebuffer);
    depth_family.recordDraw(triangle, {resourceSet});
    depth_family.endRecording();

    shadow_texture->set(depth_framebuffer);

    cameraUniform.set(renderMatrix);

    family.startRecording();
    family.recordDraw(triangle, {resourceSet});
    family.endRecording();

    wing.present();

    win.sleepMilliseconds(30);
    
    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)) {
      break;
    }

  }

  wing.destroy(family);
  wing.destroy(depth_family);

  wing.destroy(depth_framebuffer);
  wing.destroy(depth_shader);
  wing.destroy(depth_pipeline);

  wing.destroy(shadow_texture);
  
  wing.destroy(vertex_shader);
  wing.destroy(fragment_shader);
  
  wing.destroy(resourceSet);
  wing.destroy(pipeline);
  
  wing.destroy(position_buffer);
  wing.destroy(color_buffer);
  wing.destroy(index_buffer);

  wing.destroy(cameraUniform);
}
