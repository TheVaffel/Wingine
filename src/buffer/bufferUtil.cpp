#include "./bufferUtil.hpp"

#include "../memory/memoryUtil.hpp"

namespace wg::internal::bufferUtil {

    vk::Buffer createBuffer(uint32_t byte_size,
                            const vk::BufferUsageFlags& usage,
                            const vk::Device& device) {
        vk::BufferCreateInfo bci;
        bci.setSize(byte_size)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setUsage(usage);

        return device.createBuffer(bci);
    }

    vk::Buffer createStorageBuffer(uint32_t byte_size, const vk::Device& device) {
        return createBuffer(byte_size,
                            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                            device);
    }

    vk::Buffer createHostVisibleBuffer(uint32_t byte_size, const vk::Device& device) {
        return createBuffer(byte_size,
                            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc,
                            device);
    }
};
