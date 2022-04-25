#include "./bufferUtil.hpp"

#include "../memory/memoryUtil.hpp"

namespace wg::internal::bufferUtil {
    vk::Buffer createHostVisibleBuffer(uint32_t byte_size, const vk::Device& device) {
        vk::BufferCreateInfo bci;
        bci.setSize(byte_size)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setUsage(vk::BufferUsageFlagBits::eTransferDst);

        return device.createBuffer(bci);
    }
};
