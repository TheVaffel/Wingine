#include "./bufferUtil.hpp"

#include "../memory/memoryUtil.hpp"
#include "../util/math.hpp"

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

    vk::MappedMemoryRange getMappedMemoryRangeForCopy(uint32_t copy_size,
                                                      uint32_t offset,
                                                      uint32_t alignment_requirement,
                                                      uint32_t allocated_byte_size,
                                                      const vk::DeviceMemory& memory) {
        // Space to map after offset
        uint32_t map_size_top_side = math::roundUpMultiple(copy_size, alignment_requirement);

        // ensure map offset is multiple of required size multiple
        uint32_t map_offset = math::roundDownMultiple(offset, alignment_requirement);

        // Space to map before offset (0 if offset is aligned sufficiently)
        uint32_t map_size_down_side = offset - map_offset;
        uint32_t map_size = map_size_down_side + map_size_top_side;
        map_size = std::min(map_size, allocated_byte_size - map_offset);

        vk::MappedMemoryRange range;
        range.setMemory(memory)
            .setOffset(map_offset)
            .setSize(map_size);

        return range;
    }
};
