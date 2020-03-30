#include <Winval.hpp>

#include <Wingine.hpp>
#include <WgUtils.hpp>

#include <spurv.hpp>

int main() {

  const int width = 1280, height = 720;
  Winval win(width, height);

  win.setTitle("Wingine - Depth Example");
  wg::Wingine wing(width, height, win.getWinProp0(), win.getWinProp1());
  
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


  wg::Uniform cameraUniform = wing.createUniform<falg::Mat4>();
  wg::Uniform lightUniform = wing.createUniform<falg::Mat4>();
  
  // Initialize resource set layout
  
  std::vector<uint64_t> resourceSetLayout = {wg::resUniform | wg::shaVertex};
  
  wg::ResourceSet resourceSet = wing.createResourceSet(resourceSetLayout);
  resourceSet.set({&cameraUniform});

  wg::ResourceSet lightSet = wing.createResourceSet(resourceSetLayout);
  lightSet.set({&lightUniform});
  
  std::vector<wg::VertexAttribDesc> vertAttrDesc =
    std::vector<wg::VertexAttribDesc> {{wg::tFloat32, // Component type
					0, // Binding no.
					4, // Number of elements
					4 * sizeof(float), // Stride (in bytes)
					0}, // Offset (bytes)
				    {wg::tFloat32, 1, 4, 4 * sizeof(float), 0}};
  
  // Some random size
  const uint32_t shadow_buffer_width = 2000,
    shadow_buffer_height = 2000;


  wg::Framebuffer depth_framebuffer = wing.createFramebuffer(shadow_buffer_width,
							     shadow_buffer_height, true, true);
  wg::Texture shadow_texture = wing.createTexture(shadow_buffer_width,
						  shadow_buffer_height, true);

  // Initialize texture set layout

  std::vector<uint64_t> lightTextureSetLayout = {wg::resTexture | wg::shaFragment,
						 wg::resUniform | wg::shaFragment};
  
  wg::ResourceSet lightTextureSet = wing.createResourceSet(lightTextureSetLayout);
  lightTextureSet.set({shadow_texture, &lightUniform});
  
  std::vector<uint32_t> depth_vertex_shader;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_VERTEX, vec4_s> shader;
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
						    {&depth_shader}, true,
						    shadow_buffer_width, shadow_buffer_height);
  
  
  std::vector<uint32_t> vertex_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_VERTEX, vec4_s, vec4_s> shader;
    vec4_v s_pos = shader.input<0>();
    vec4_v s_col = shader.input<1>();

    SUniformBinding<mat4_s> trans_bind = shader.uniformBinding<mat4_s>(0, 0);
    mat4_v trans = trans_bind.member<0>();

    vec4_v transformed_pos = trans * s_pos;
    vec4_v world_pos = s_pos; // Should be model transformed, but doesn't have one in this example
    
    shader.setBuiltin<BUILTIN_POSITION>(transformed_pos);
    shader.compile(vertex_spirv, s_col, world_pos);
  }

  wg::Shader vertex_shader = wing.createShader(wg::shaVertex, vertex_spirv);

  
  std::vector<uint32_t> fragment_spirv;
  {
    using namespace spurv;

    SShader<SShaderType::SHADER_FRAGMENT, vec4_s, vec4_s> shader;
    vec4_v in_col = shader.input<0>();
    vec4_v in_wpos = shader.input<1>();

    texture2D_v shadow_tex = shader.uniformBinding<texture2D_s>(1, 0);
    SUniformBinding<mat4_s> shadow_trans_bind = shader.uniformBinding<mat4_s>(1, 1);
    mat4_v shadow_trans = shadow_trans_bind.member<0>();

    vec4_v light_pos = shadow_trans * in_wpos;

    vec2_v light_pos2d = (vec2_s::cons(light_pos[0] / light_pos[3],
				       light_pos[1] / light_pos[3]) +
				       vec2_s::cons(1.0, 1.0)) * 0.5;

    float_v lookupval = shadow_tex[light_pos2d][0];

    float_v intensity = select(lookupval + ( 1e-5) >= light_pos[2] / light_pos[3], 
        1.f, 0.2f);

    // float_v intensity = lookupval;
    vec4_v mul = intensity * in_col;

    vec4_v res = vec4_s::cons(mul[0], mul[1], mul[2], 1.0f);

    shader.compile(fragment_spirv, res);
  }

  wg::Shader fragment_shader = wing.createShader(wg::shaFragment, fragment_spirv);

  wg::Pipeline pipeline = wing.
    createPipeline(vertAttrDesc,
		   {&resourceSetLayout, &lightTextureSetLayout},
		   {&vertex_shader, &fragment_shader});

  
  wg::RenderFamily family = wing.createRenderFamily(pipeline, true);

  wg::RenderFamily depth_family = wing.createRenderFamily(depth_pipeline, true);

  wgut::Camera camera(F_PI / 3.f, 9.0 / 16.0, 0.01f, 100.0f);
  camera.setLookAt(falg::Vec3(2.0f, 2.0f, 2.0f),
		   falg::Vec3(0.0f, 0.0f, -2.5f),
		   falg::Vec3(0.0f, 1.0f, 0.0f));

  wgut::Camera light_camera(F_PI / 3.f, 1.0f, 0.01f, 100.0f);
  light_camera.setLookAt(falg::Vec3(-1.0f, 3.0f, -1.0f),
			 falg::Vec3(0.0f, 0.0f, -2.5f),
			 falg::Vec3(0.0f, 1.0f, 0.0f));

  while (win.isOpen()) {
    
    lightUniform.set(light_camera.getRenderMatrix());
    
    falg::Mat4 renderMatrix = camera.getRenderMatrix();

    depth_family.startRecording(depth_framebuffer);
    depth_family.recordDraw({&position_buffer, &color_buffer}, index_buffer, {lightSet});
    depth_family.endRecording();

    shadow_texture->set(depth_framebuffer);

    cameraUniform.set(renderMatrix);
    family.startRecording();
    family.recordDraw({&position_buffer, &color_buffer}, index_buffer, {resourceSet, lightTextureSet});
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

  wing.destroy(lightTextureSet);
  wing.destroy(lightSet);
  wing.destroy(resourceSet);
  wing.destroy(pipeline);
  
  wing.destroy(position_buffer);
  wing.destroy(color_buffer);
  wing.destroy(index_buffer);

  wing.destroy(cameraUniform);
  wing.destroy(lightUniform);
}
