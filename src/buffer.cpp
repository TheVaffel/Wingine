#include "buffer.hpp"
#include "util.hpp"

#include "Wingine.hpp"

namespace wg {
  
  Buffer::Buffer(Wingine& wing,
		 vk::BufferUsageFlags usage,
		 uint32_t size,
		 bool host_updatable) {
    this->wing = &wing;
    
    vk::Device device = wing.getDevice();
    
    this->host_updatable = host_updatable;
    
    vk::BufferCreateInfo bci;
    bci.setSize(size)
      .setSharingMode(vk::SharingMode::eExclusive)
      .setUsage(usage |
		vk::BufferUsageFlagBits::eTransferDst);
    
    this->buffer = device.createBuffer(bci);

    vk::MemoryRequirements memReqs =
      device.getBufferMemoryRequirements(this->buffer);

    vk::MemoryAllocateInfo mai;
    mai.setAllocationSize(memReqs.size);

    if (host_updatable) {
      mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
						    vk::MemoryPropertyFlagBits::eHostCoherent |
						    vk::MemoryPropertyFlagBits::eHostVisible,
						    wing.device_memory_props));

      this->memory = device.allocateMemory(mai);
    } else {
      mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
						    vk::MemoryPropertyFlagBits::eDeviceLocal,
						    wing.device_memory_props));
      
      this->memory = device.allocateMemory(mai);
      
      bci.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
      
      this->update_buffer = device.createBuffer(bci);

      memReqs = device.getBufferMemoryRequirements(this->update_buffer);

      mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
						    vk::MemoryPropertyFlagBits::eHostCoherent |
						    vk::MemoryPropertyFlagBits::eHostVisible,
						    wing.device_memory_props));
      this->update_memory = device.allocateMemory(mai);

      device.bindBufferMemory(this->update_buffer, this->update_memory, 0);
    }

    
    device.bindBufferMemory(this->buffer, this->memory, 0); // Buffer, memory, memoryOffset
  }

  void Buffer::set(void* data, uint32_t sizeInBytes, uint32_t offsetInBytes) {
    void* mapped;
    vk::Device device = this->wing->getDevice();
    
    if (this->host_updatable) {
      device.mapMemory(this->memory, offsetInBytes, sizeInBytes, {}, &mapped);

      memcpy(mapped, data, sizeInBytes);

      device.unmapMemory(this->memory);
    } else {      

      vk::Device device = this->wing->getDevice();
      Command command = this->wing->getCommand();
      vk::Queue queue = this->wing->getGraphicsQueue();
    
      
      device.mapMemory(this->update_memory, offsetInBytes, sizeInBytes, {}, &mapped);

      memcpy(mapped, data, sizeInBytes);

      device.unmapMemory(this->update_memory);

      vk::BufferCopy bc;
      bc.setSrcOffset(offsetInBytes)
	.setDstOffset(offsetInBytes)
	.setSize(sizeInBytes);
      
      vk::CommandBufferBeginInfo cbbi;

      command.buffer.begin(cbbi);

      command.buffer.copyBuffer(this->update_buffer,
				this->buffer,
				1, &bc);
      
      command.buffer.end();

      vk::SubmitInfo si;
      si.setCommandBufferCount(1)
	.setPCommandBuffers(&command.buffer);

      device.waitForFences(1, &command.fence, VK_TRUE,
			   (uint64_t)1e9);
      device.resetFences(1, &command.fence);
      
      queue.submit(1, &si, command.fence);

      device.waitForFences(1, &command.fence, VK_TRUE,
			   (uint64_t)1e9);
    }
  }

  IndexBuffer::IndexBuffer(Wingine& wing,
			   int numIndices,
			   bool host_updatable) :
    Buffer(wing,
	   vk::BufferUsageFlagBits::eIndexBuffer,
	   numIndices * sizeof(uint32_t),
	   host_updatable) {
    this->num_indices = numIndices;
  }

  void IndexBuffer::set(uint32_t* indices,
			uint32_t num,
			uint32_t offsetElements) {
    Buffer::set((void*)indices,
		num * sizeof(uint32_t),
		offsetElements * sizeof(uint32_t));
  }

  int IndexBuffer::getNumIndices() const {
    return this->num_indices;
  }

  
  _VertexBuffer::_VertexBuffer(Wingine& wing,
			       vk::BufferUsageFlagBits bit,
			       int size,
			       bool host_updatable) :
    Buffer(wing,
	   bit,
	   size,
	   host_updatable) { }

};
