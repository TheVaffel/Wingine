#ifndef __WINGINE_HPP
#define __WINGINE_HPP

#include <iostream>

#include <vector>
#include <map> 

#define DEBUG

#include <Winval.hpp>

#include "declarations.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "framebuffer.hpp"
#include "resource.hpp"
#include "pipeline.hpp"
#include "renderfamily.hpp"
#include "util.hpp"


namespace wg {

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
    vk::Semaphore image_acquired_semaphore;
    vk::Semaphore image_drawn_semaphore;

    uint32_t current_swapchain_image;
    std::vector<Framebuffer*> framebuffers;
    
    vk::DescriptorPool descriptor_pool;

    vk::PipelineCache pipeline_cache;
    
    vk::Queue graphics_queue,
      present_queue,
      compute_queue;

    int32_t present_queue_index,
      graphics_queue_index,
      compute_queue_index;

    vk::CommandPool present_command_pool,
        graphics_command_pool,
      compute_command_pool;

    Command present_command,
      compute_command;

    Command general_purpose_command;

    uint32_t window_width, window_height;

    std::map<std::vector<uint64_t>, ResourceSetLayout> resourceSetLayoutMap;
    std::map<RenderPassType, vk::RenderPass> compatibleRenderPassMap;
    
    void init_vulkan(int width, int height,
		     winval_type_0 arg0,
		     winval_type_1 arg1,
		     const char* str);
    
    void init_instance(int width, int height, const char* str);
    void init_surface(winval_type_0 arg0, winval_type_1 arg1);
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
			  vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
			  vk::ImageLayout layout = vk::ImageLayout::eUndefined);
			  
    void cons_image_memory(Image& image,
			   vk::MemoryPropertyFlags memProps);
    void cons_image_view(Image& image,
			 ImageViewType type);

    void copy_image(uint32_t w1, uint32_t h1, vk::Image src,
		   vk::ImageLayout srcCurrentLayout, vk::ImageLayout srcFinalLayout,
		   uint32_t w2, uint32_t h2, vk::Image dst,
		   vk::ImageLayout dstCurrentLayout, vk::ImageLayout dstFinalLayout,
		    vk::ImageAspectFlagBits flag,
		    vk::Semaphore *wait_semaphore = nullptr);

    void cmd_set_layout(vk::CommandBuffer& commandBuffer, vk::Image image,
			vk::ImageAspectFlagBits aspect, vk::ImageLayout currentLayout,
			vk::ImageLayout finalLayout);
    
    // Don't delete color images, those are handled by swapchain
    void destroySwapchainFramebuffer(Framebuffer* framebuffer);
    void destroySwapchainImage(Image& image);

    vk::Queue getGraphicsQueue();
    vk::Device getDevice();
    Command getCommand();
    vk::DescriptorPool getDescriptorPool();
    vk::CommandPool getPresentCommandPool();
    vk::CommandPool getGraphicsCommandPool();
    
    std::vector<Framebuffer*>& getFramebuffers();

    void register_compatible_render_pass(RenderPassType type);
    vk::RenderPass create_render_pass(RenderPassType type,
					       bool clear);

    void destroy(Image& image);

  public:

    Framebuffer* getCurrentFramebuffer();
    int getCurrentFramebufferIndex();
    int getNumFramebuffers();
    
    int getWindowWidth();
    int getWindowHeight();

    template<typename Type>
    VertexBuffer<Type>* createVertexBuffer(uint32_t num, bool host_updatable = true);

    IndexBuffer* createIndexBuffer(uint32_t num_indices);

    template<typename Type>
    Uniform<Type>* createUniform();

    StorageBuffer* createStorageBuffer(uint32_t num_bytes, bool host_updatable = true);

    RenderFamily* createRenderFamily(Pipeline* pipeline, bool clear, int num_framebuffers = 0);
      
    ResourceSet* createResourceSet(std::vector<uint64_t>& resourceLayout);

    Shader* createShader(uint64_t stage_bit, std::vector<uint32_t>& spirv);

    Pipeline* createPipeline(const std::vector<VertexAttribDesc>& descriptions,
			     const std::vector<std::vector<uint64_t> >& resourceSetLayout,
			     const std::vector<Shader*>& shaders,
			     bool depthOnly = false,
			     int width = -1, int height = -1);
    
    Framebuffer* createFramebuffer(uint32_t width, uint32_t height,
				    bool depthOnly = false, bool withoutSemaphore = false);

    Texture* createTexture(uint32_t width, uint32_t height, bool depth = false);
    
    void present();

    void destroy(Pipeline* pipeline);
    void destroy(RenderFamily* family);
    void destroy(Buffer* buffer);
    void destroy(Shader* shader);
    void destroy(Texture* texture);
    void destroy(StorageBuffer* storageBuffer);
    
    template<typename Type>
    void destroy(Uniform<Type>* uniform);
    
    void destroy(Framebuffer* framebuffer);
    
    Wingine(Winval& win);
  
    Wingine(int width, int height,
	    winval_type_0 arg0,
	    winval_type_1 arg1,
	    const char* str = "Wingine");
  

    ~Wingine();

    friend class Buffer;
    friend class Pipeline;
    friend class Framebuffer;
    friend class RenderFamily;
    friend class ResourceSetLayout;
    friend class ResourceSet;
    friend class Texture;
  };


  // Template things


  template<typename Type>
  VertexBuffer<Type>* Wingine::createVertexBuffer(uint32_t num, bool host_updatable) {
    return new VertexBuffer<Type>(*this, num, host_updatable);
  }

  template<typename Type>
  Uniform<Type>* Wingine::createUniform() {
    return new Uniform<Type>(*this);
  }

  
  template<typename Type>
  void Wingine::destroy(Uniform<Type>* uniform) {
    this->destroy(uniform->buffer);
    delete uniform;
  }
  
  
};

#endif // __WINGINE_HPP
