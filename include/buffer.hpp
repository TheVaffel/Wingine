#ifndef WG_BUFFER_HPP
#define WG_BUFFER_HPP

#include "declarations.hpp"

namespace wg {
  
  
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

  // Template declarations
  
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
};

#endif // WG_BUFFER_HPP
