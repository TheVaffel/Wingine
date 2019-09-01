#ifndef __WINGINE_HPP
#define __WINGINE_HPP

#include <Winval.hpp>


#ifdef WIN32

typedef HINST _win_arg_type_0;
typedef HWND _win_arg_type_1;

#define VK_USE_PLATFORM_WIN32_KHR

#else // WIN32

typedef Window _win_arg_type_0;
typedef Display* _win_arg_type_1;

#define VK_USE_PLATFORM_XLIB_KHR

#endif // WIN32

#include <vulkan/vulkan.hpp>

#include <vector>
#include <map>

#define DEBUG

namespace wg {

  enum ResourceType {
    resUniform = (uint64_t)VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    resTexture = (uint64_t)VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    resStoreImage = (uint64_t)VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
  };

  // These are shifted, because we want to combine them
  // with resource types as a bitmask
  enum ShaderStage {
    shaVertex = (uint64_t)VK_SHADER_STAGE_VERTEX_BIT << 32, 
    shaFragment = (uint64_t)VK_SHADER_STAGE_FRAGMENT_BIT << 32
  };
  
  enum ImageViewType {
    wImageViewColor,
    wImageViewDepth
  };

  enum RenderPassType {
    renColorDepth,
    renDepth
  };

  class Wingine;
  class Resource;
  class RenderFamily;

  template<typename Type>
  class Uniform;
  
  class Buffer {
    Wingine* wing;
    
    vk::Buffer buffer;
    vk::DeviceMemory memory;

    bool host_updatable;
    
    vk::Buffer update_buffer;
    vk::DeviceMemory update_memory;
    
  protected:
    Buffer(Wingine& wing,
	   vk::BufferUsageFlags flags,
	   uint32_t size,
	   bool host_updatable = true); // True means faster to update, slower to use
    void set(void* data, uint32_t sizeInBytes, uint32_t offsetInBytes = 0);

    template<typename Type>
    friend class Uniform;
    friend class RenderFamily;
    friend class Wingine;
  };

  class _VertexBuffer : public Buffer {
  protected:
    _VertexBuffer(Wingine& wing,
		  vk::BufferUsageFlagBits bit,
		  int num,
		  bool host_updatable = true);

    friend class Wingine;
    friend class Resource;
    template<typename Type>
    friend class Uniform;
  };
  
  template<typename Type>
  class VertexBuffer : public _VertexBuffer {
    VertexBuffer(Wingine& wing,
		 int num,
		 bool host_updatable = true);

  public:
    void set(Type* t, uint32_t num, uint32_t offsetElements = 0);

    friend class Wingine;
    friend class Resource;
  };

  class IndexBuffer : public Buffer {
    int num_indices;
    IndexBuffer(Wingine& wing,
		int numIndices,
		bool host_updatable = false);

  public:
    void set(uint32_t* indices,
	     uint32_t num, uint32_t offsetElements = 0);

    friend class RenderFamily;
    friend class Wingine;
  };

  class RenderObject {
    std::vector<_VertexBuffer*> vertexBuffers;
    IndexBuffer indexBuffer;
    
  public:
    RenderObject(const std::vector<_VertexBuffer*>& buffers,
		 IndexBuffer indexBuffer);
    
    friend class RenderFamily;
  };

  class ResourceSetLayout {
    Wingine* wing;
    vk::DescriptorSetLayout layout;

    
    ResourceSetLayout(Wingine& wing,
		      const std::vector<uint64_t>& ll);

  public:
    ResourceSetLayout();
    
    friend class ResourceSet;
    friend class Wingine;
    friend class Pipeline;
  };
  
  class Image {
    
    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;

  public:

    const vk::Image& getImage() const;
    const vk::DeviceMemory& getMemory() const;
    const vk::ImageView& getView() const;

    friend class Wingine;
  };

  
  class ResourceSet {
    vk::Device device;
    vk::DescriptorSet descriptor_set;

    ResourceSet(Wingine& wing, vk::DescriptorSetLayout layout);

  public:
    void set(const std::vector<Resource*>& resources);

    friend class RenderFamily;
    friend class Wingine;
  };
  
  class Resource {
  protected:
    vk::DescriptorType type;
    vk::DescriptorImageInfo* image_info;
    vk::DescriptorBufferInfo* buffer_info;

    Resource(vk::DescriptorType type);

    friend class ResourceSet;
    
    friend class Wingine;
  };

  template<typename Type>
  class Uniform : public Resource {
    Buffer buffer;
    
    Uniform(Wingine& wing);
    
  public:
    void set(Type t);

    friend class Wingine;
  };

  enum ComponentType {
    tFloat32,
    tFloat64,
    tInt32,
    tInt64
  };
  
  struct VertexAttribDesc {
    ComponentType component_type;
    uint32_t binding_num;
    uint32_t num_elements;
    uint32_t stride_in_bytes;
    uint32_t offset_in_bytes;
  };

  class Shader {
    vk::PipelineShaderStageCreateInfo shader_info;
    
    Shader(vk::Device& device,
	   uint64_t stage,
	   std::vector<uint32_t>& spirv);

    friend class Pipeline;
    friend class Wingine;
  };
  
  
  class Framebuffer {
    Image colorImage;
    Image depthImage;
    vk::Framebuffer framebuffer;

  public:

    void destroy();
    
    const Image& getColorImage() const;
    const Image& getDepthImage() const;
    const vk::Framebuffer& getFramebuffer() const;

    friend class Wingine;
  };

  struct Command {
    vk::CommandBuffer buffer;
    vk::Fence fence;
  };


  // Future addition
  struct PipelineSetup {
    bool clearScreen = true;
  };
  
  
  class Pipeline {
    vk::Pipeline pipeline;
    vk::PipelineLayout layout;
    RenderPassType render_pass_type;
    
    Pipeline(Wingine& wing,
	     int width, int height,
	     std::vector<VertexAttribDesc>& descriptions,
	     std::vector<ResourceSetLayout>& resourceSetLayout,
	     std::vector<Shader*>& shaders,
	     bool depthOnly);

    friend class RenderFamily;
    friend class Wingine;
  };

  class RenderFamily {
    Wingine* wing;

    Command command;
    Pipeline* pipeline;
    vk::RenderPass render_pass;
    bool clears;
    
    RenderFamily(Wingine& wing,
		 Pipeline& pipeline,
		 bool clear);

    void submit_command();
    
  public:
    void startRecording();
    void recordDraw(RenderObject& obj,
		    std::vector<ResourceSet> sets);
    void endRecording();
    
    friend class Wingine;
  };
  
  class Wingine {

    vk::Instance vulkan_instance;
    vk::DispatchLoaderDynamic dispatcher;

    vk::DebugReportCallbackEXT debug_callback;

    vk::SurfaceKHR surface;
    vk::PhysicalDevice physical_device;
    vk::PhysicalDeviceMemoryProperties device_memory_props;
    vk::Device device;

    vk::Format surface_format;
    vk::SwapchainKHR swapchain;

    std::vector<vk::Image> swapchain_images;
    vk::Fence image_acquired_fence;
    vk::Semaphore image_acquired_semaphore;
    vk::Semaphore draw_semaphore;

    uint32_t current_swapchain_image;
    std::vector<Framebuffer> framebuffers;
    
    vk::DescriptorPool descriptor_pool;

    vk::PipelineCache pipeline_cache;
    
    vk::Queue graphics_queue,
      present_queue,
      compute_queue;

    int32_t present_queue_index,
      graphics_queue_index,
      compute_queue_index;

    vk::CommandPool present_command_pool,
      compute_command_pool;

    Command present_command,
      compute_command;

    Command general_purpose_command;
    
    uint32_t window_width, window_height;

    std::map<std::vector<uint64_t>, ResourceSetLayout> resourceSetLayoutMap;
    std::map<RenderPassType, vk::RenderPass> compatibleRenderPassMap;
    
    void init_vulkan(int width, int height,
		     _win_arg_type_0 arg0,
		     _win_arg_type_1 arg1,
		     const char* str);
    
    void init_instance(int width, int height, const char* str);
    void init_surface(_win_arg_type_0 arg0, _win_arg_type_1 arg1);
    void init_device();
    void init_command_buffers();
    void init_swapchain();
    void init_generic_render_pass();
    void init_framebuffers();
    void init_descriptor_pool();
    void init_pipeline_cache();
    void stage_next_image();
    
    void cons_image_image(Image& image,
			  uint32_t width, uint32_t height,
			  vk::Format format,
			  vk::ImageUsageFlags usage,
			  vk::ImageTiling tiling = vk::ImageTiling::eOptimal);
			  
    void cons_image_memory(Image& image,
			   vk::MemoryPropertyFlagBits memProps);
    void cons_image_view(Image& image,
			 ImageViewType type);

    // Don't delete color images, those are handled by swapchain
    void destroySwapchainFramebuffer(Framebuffer& framebuffer);
    void destroySwapchainImage(Image& image);

    vk::Queue getGraphicsQueue();
    vk::Device getDevice();
    Command getCommand();
    vk::DescriptorPool getDescriptorPool();
    vk::CommandPool getDefaultCommandPool();
    vk::Framebuffer getCurrentFramebuffer();

    void register_compatible_render_pass(RenderPassType type);
    vk::RenderPass create_render_pass(RenderPassType type,
					       bool clear);
    
  public:

    template<typename Type>
    VertexBuffer<Type> createVertexBuffer(uint32_t num, bool host_updatable = true);

    IndexBuffer createIndexBuffer(uint32_t num_indices);

    template<typename Type>
    Uniform<Type> createUniform();

    RenderFamily createRenderFamily(Pipeline& pipeline, bool clear);
      
    ResourceSet createResourceSet(std::vector<uint64_t>& resourceLayout);

    Shader createShader(uint64_t stage_bit, std::vector<uint32_t>& spirv);

    Pipeline createPipeline(std::vector<VertexAttribDesc>& descriptions,
			    std::vector<std::vector<uint64_t>* > resourceSetLayout,
			    std::vector<Shader*> shaders,
			    bool depthOnly = false) ;
    
    void present();

    void destroy(Pipeline& pipeline);
    void destroy(RenderFamily& family);
    void destroy(ResourceSet& set);
    void destroy(Buffer& buffer);
    void destroy(Shader& shader);
    
    template<typename Type>
    void destroy(Uniform<Type>& uniform);
    
    void destroy(Image& image);
    void destroy(Framebuffer& framebuffer);
    
    Wingine(Winval& win);
  
    Wingine(int width, int height,
	    _win_arg_type_0 arg0,
	    _win_arg_type_1 arg1,
	    const char* str = "Wingine");
  

    ~Wingine();

    friend class Buffer;
    friend class Pipeline;
    friend class RenderFamily;
    friend class ResourceSetLayout;
    friend class ResourceSet;
  };


  // Template things
  
  template<typename Type>
  VertexBuffer<Type>::VertexBuffer(Wingine& wing,
				   int num,
				   bool host_updatable) :
    _VertexBuffer(wing,
		  vk::BufferUsageFlagBits::eVertexBuffer,
		  num * sizeof(Type),
		  host_updatable) { }

  template<typename Type>
  void VertexBuffer<Type>::set(Type* t, uint32_t num, uint32_t offsetElements) {
    Buffer::set((void*)t, num * sizeof(Type), offsetElements * sizeof(Type));
  }

  template<typename Type>
  Uniform<Type>::Uniform(Wingine& wing) : Resource(vk::DescriptorType::eUniformBuffer),
					  buffer(wing,
						 vk::BufferUsageFlagBits::eUniformBuffer,
						 sizeof(Type),
						 true)
  {
    
    this->buffer_info = new vk::DescriptorBufferInfo();
    this->buffer_info->buffer = this->buffer.buffer;
    this->buffer_info->offset = 0;
    this->buffer_info->range = sizeof(Type);
  }


  template<typename Type>
  VertexBuffer<Type> Wingine::createVertexBuffer(uint32_t num, bool host_updatable) {
    return VertexBuffer<Type>(*this, num, host_updatable);
  }

  template<typename Type>
  Uniform<Type> Wingine::createUniform() {
    return Uniform<Type>(*this);
  }

  template<typename Type>
  void Uniform<Type>::set(Type t) {
    this->buffer.set(&t, sizeof(t), 0);
  }

  
  template<typename Type>
  void Wingine::destroy(Uniform<Type>& uniform) {
    this->destroy(uniform.buffer);
  }
  
};
  
#endif // __WINGINE_HPP
