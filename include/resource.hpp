#ifndef WG_RESOURCE_HPP
#define WG_RESOURCE_HPP

#include "declarations.hpp"
#include "image.hpp"
#include "buffer.hpp"

namespace wg {
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


  class Texture : public Image, public Resource {
    Wingine* wing;
    
    vk::Sampler sampler;

    vk::Image staging_image;
    vk::DeviceMemory staging_memory;
    vk::MemoryRequirements staging_memory_memreq;

    vk::ImageLayout current_staging_layout;
    vk::ImageAspectFlagBits aspect;
    
    uint32_t stride_in_bytes;

    Texture(Wingine& wing,
	     uint32_t width, uint32_t height,
	     bool depth);
  public:

    // Returns stride in bytes
    uint32_t getStride();
    
    void set(unsigned char* pixels,
	     const std::initializer_list<SemaphoreChain*>& wait_semaphores,
	     bool fixed_stride = false);
    void set(Framebuffer* framebuffer,
	     const std::initializer_list<SemaphoreChain*>& wait_semaphores);

    friend class Wingine;
  };

  
  class StorageBuffer : public Resource {

    int num_bytes;
    Buffer* buffer;
    StorageBuffer(Wingine& wing,
		  int size_bytes,
		  bool host_updatable);
  public:
    void set(void* data,
	     uint32_t num_bytes, uint32_t offset_bytes = 0);

    friend class Wingine;
  };

  // Template declarations
  
  template<typename Type>
  class Uniform : public Resource {
    Buffer* buffer;
    
    Uniform(Wingine& wing);
    
  public:
    void set(Type t);

    friend class Wingine;
  };

  template<typename Type>
  void Uniform<Type>::set(Type t) {
    this->buffer->set(&t, sizeof(t), 0);
  }

  template<typename Type>
  Uniform<Type>::Uniform(Wingine& wing) : Resource(vk::DescriptorType::eUniformBuffer),
					  buffer(new Buffer(wing,
							    (vk::BufferUsageFlags)(vk::BufferUsageFlagBits::eUniformBuffer),
							    sizeof(Type),
							    true))
  {
    
    this->buffer_info = new vk::DescriptorBufferInfo();
    this->buffer_info->buffer = this->buffer->buffer;
    this->buffer_info->offset = 0;
    this->buffer_info->range = sizeof(Type);
  }
};

#endif // WG_RESOURCE_HPP
