#include "./buffer.hpp"
#include "./util.hpp"
#include "./log.hpp"

#include "./Wingine.hpp"

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
                                                          wing.device_manager->getDeviceMemoryProperties()));

            this->memory = device.allocateMemory(mai);
        } else {
            mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
                                                          vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                          wing.device_manager->getDeviceMemoryProperties()));

            this->memory = device.allocateMemory(mai);

            bci.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

            this->update_buffer = device.createBuffer(bci);

            memReqs = device.getBufferMemoryRequirements(this->update_buffer);

            mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
                                                          vk::MemoryPropertyFlagBits::eHostCoherent |
                                                          vk::MemoryPropertyFlagBits::eHostVisible,
                                                          wing.device_manager->getDeviceMemoryProperties()));
            this->update_memory = device.allocateMemory(mai);

            device.bindBufferMemory(this->update_buffer, this->update_memory, 0);
        }


        device.bindBufferMemory(this->buffer, this->memory, 0); // Buffer, memory, memoryOffset
    }

    void* Buffer::_mapMemory() {
        void* data;

        vk::Device device = this->wing->getDevice();

        _wassert(this->host_updatable, "must have host_updatable set for memory to be mappable");

        _wassert_result(device.mapMemory(this->memory, 0, VK_WHOLE_SIZE, {}, &data));

        return data;
    }

    void Buffer::_unmapMemory() {
        vk::Device device = this->wing->getDevice();
        device.unmapMemory(this->memory);
    }

    void Buffer::set(const void* data, uint32_t sizeInBytes, uint32_t offsetInBytes) {
        void* mapped;
        vk::Device device = this->wing->getDevice();

        if (this->host_updatable) {
            _wassert_result(device.mapMemory(this->memory, offsetInBytes, sizeInBytes, {}, &mapped),
                            "map memory in Buffer::set");

            memcpy(mapped, data, sizeInBytes);

            device.unmapMemory(this->memory);
        } else {

            vk::Device device = this->wing->getDevice();
            Command command = this->wing->getCommand();
            vk::Queue queue = this->wing->getGraphicsQueue();


            _wassert_result(device.mapMemory(this->update_memory, offsetInBytes, sizeInBytes, {}, &mapped),
                            "map update memory in Buffer::set");

            memcpy(mapped, data, sizeInBytes);

            device.unmapMemory(this->update_memory);

            vk::BufferCopy bc;
            bc.setSrcOffset(offsetInBytes)
                .setDstOffset(offsetInBytes)
                .setSize(sizeInBytes);

            vk::CommandBufferBeginInfo cbbi;

            _wassert_result(device.waitForFences(1, &command.fence, VK_TRUE,
                                                 (uint64_t)1e9),
                            "wait for fence 1 in Buffer::set");

            command.buffer.begin(cbbi);

            command.buffer.copyBuffer(this->update_buffer,
                                      this->buffer,
                                      1, &bc);

            command.buffer.end();

            vk::SubmitInfo si;
            si.setCommandBufferCount(1)
                .setPCommandBuffers(&command.buffer);

            _wassert_result(device.resetFences(1, &command.fence),
                            "reset fence in Buffer::set");

            _wassert_result(queue.submit(1, &si, command.fence),
                            "submit command in Buffer::set");

            _wassert_result(device.waitForFences(1, &command.fence, VK_TRUE,
                                                 (uint64_t)1e9),
                            "wait for fence 2 in Buffer::set");
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

    void IndexBuffer::set(const uint32_t* indices,
                          uint32_t num,
                          uint32_t offsetElements) {
        Buffer::set((const void*)indices,
                    num * sizeof(uint32_t),
                    offsetElements * sizeof(uint32_t));
    }

    uint32_t* IndexBuffer::mapMemory() {
        return reinterpret_cast<uint32_t*>(this->_mapMemory());
    }

    void IndexBuffer::unmapMemory() {
        this->_unmapMemory();
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
