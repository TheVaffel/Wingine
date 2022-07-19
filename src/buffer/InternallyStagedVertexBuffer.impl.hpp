#pragma once

#include "./InternallyStagedVertexBuffer.hpp"
#include "../memory/memoryUtil.hpp"
#include "./copyBuffer.hpp"
#include "../sync/fenceUtil.hpp"

#include "../util/log.hpp"
#include "../util/math.hpp"

#include "./bufferUtil.hpp"

namespace wg::internal {

    template<typename T>
    InternallyStagedVertexBuffer<T>::InternallyStagedVertexBuffer(uint32_t num_elements,
                                                                  std::shared_ptr<DeviceManager> device_manager,
                                                                  std::shared_ptr<QueueManager> queue_manager,
                                                                  std::shared_ptr<CommandManager> command_manager)
        : BasicBuffer(num_elements * sizeof(T),
                      vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                      device_manager),
          staging_buffer(num_elements * sizeof(T),
                         device_manager,
                         queue_manager,
                         command_manager) { }

    template<typename T>
    void InternallyStagedVertexBuffer<T>::set(const T* data, uint32_t first_element, uint32_t element_count) {
        uint32_t copy_size = element_count * sizeof(T);
        uint32_t offset = first_element * sizeof(T);
        this->staging_buffer.set(data, *this, copy_size, offset);
    }
};
