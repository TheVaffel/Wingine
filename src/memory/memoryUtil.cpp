#include "./memoryUtil.hpp"

#include "../util/math.hpp"

namespace wg::internal::memoryUtil {

    namespace {

        bool memoryIndexSatisfiesRequirements(uint32_t index,
                                              const vk::MemoryRequirements& memory_requirements) {
            return ((1 << index) & memory_requirements.memoryTypeBits) != 0;
        }

        bool memoryTypeHasFlags(const vk::MemoryType& memory_type,
                                const vk::MemoryPropertyFlags& required_property_flags) {
            return (memory_type.propertyFlags & required_property_flags) == required_property_flags;
        }

        /**
         * Get index of suitable memory type among physical device's memory types.
         * The memory must both be among the types specified by memory_requirements
         * and contain all the provided required_memory_property_flags
         */
        uint32_t getMemoryTypeIndex(const vk::MemoryRequirements& memory_requirements,
                                    const vk::MemoryPropertyFlags& required_memory_property_flags,
                                    const vk::PhysicalDeviceMemoryProperties& device_memory_properties) {

            for (uint32_t type_i = 0; type_i < device_memory_properties.memoryTypeCount; type_i++) {
                if (memoryIndexSatisfiesRequirements(type_i, memory_requirements)) {
                    if (memoryTypeHasFlags(device_memory_properties.memoryTypes[type_i],
                                           required_memory_property_flags)) {
                        return type_i;
                    }
                }
            }

            throw std::runtime_error("[memoryUtil.cpp] Can't find applicable memory type index");
        }

        vk::DeviceMemory createMemory(uint32_t memory_size,
                                      uint32_t memory_type_index,
                                      const vk::Device& device) {

            vk::MemoryAllocateInfo allocate_info;
            allocate_info.allocationSize = memory_size;
            allocate_info.memoryTypeIndex = memory_type_index;
            vk::DeviceMemory memory = device.allocateMemory(allocate_info);
            return memory;
        }

        vk::DeviceMemory createMemoryForRequirements(
            const vk::MemoryRequirements& requirements,
            const vk::MemoryPropertyFlags& property_flags,
            const vk::PhysicalDeviceMemoryProperties& device_memory_properties,
            const vk::Device& device) {

            uint32_t memory_type_index = getMemoryTypeIndex(requirements,
                                                            property_flags,
                                                            device_memory_properties);

            return createMemory(requirements.size, memory_type_index, device);
        }
    };


    /*
     * Memory
     */

    vk::DeviceMemory createAndBindMemoryForImage(
        const vk::Image& image,
        const vk::MemoryPropertyFlagBits& memory_properties,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_memory_properties) {

        vk::MemoryRequirements requirements = device.getImageMemoryRequirements(image);

        vk::DeviceMemory memory = createMemoryForRequirements(
            requirements,
            memory_properties,
            device_memory_properties,
            device);

        device.bindImageMemory(image, memory, 0);

        return memory;
    }


    vk::DeviceMemory createAndBindHostAccessibleMemoryForImage(
        const vk::Image& image,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_properties) {

        vk::MemoryRequirements requirements = device.getImageMemoryRequirements(image);
        uint32_t memory_type_index = getMemoryTypeIndex(requirements,
                                                        vk::MemoryPropertyFlagBits::eHostVisible,
                                                        device_properties);

        vk::DeviceMemory memory = createMemory(requirements.size, memory_type_index, device);

        device.bindImageMemory(image, memory, 0);

        return memory;
    }

    vk::DeviceMemory createAndBindMemoryForBuffer(
        const vk::Buffer& buffer,
        const vk::MemoryPropertyFlags& memory_properties,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_memory_properties) {

        vk::MemoryRequirements requirements = device.getBufferMemoryRequirements(buffer);

        vk::DeviceMemory memory = createMemoryForRequirements(
            requirements,
            memory_properties,
            device_memory_properties,
            device);

        device.bindBufferMemory(buffer, memory, 0);
        return memory;
    }

    vk::DeviceMemory createAndBindHostAccessibleMemoryForBuffer(
        const vk::Buffer& buffer,
        const vk::Device& device,
        const vk::PhysicalDeviceMemoryProperties& device_memory_properties) {

        vk::MemoryRequirements requirements = device.getBufferMemoryRequirements(buffer);

        vk::DeviceMemory memory = createMemoryForRequirements(
            requirements,
            vk::MemoryPropertyFlagBits::eHostVisible,
            device_memory_properties,
            device);

        device.bindBufferMemory(buffer, memory, 0);

        return memory;
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

    void unmapMemory(const vk::DeviceMemory& memory,
                     const vk::Device& device) {
        vk::MappedMemoryRange range;
        range.setOffset(0)
            .setSize(VK_WHOLE_SIZE)
            .setMemory(memory);
        device.flushMappedMemoryRanges({ range });
        device.unmapMemory(memory);
    }


    void* mapRawMemory(const vk::DeviceMemory& memory,
                       const vk::Device& device) {
        return device.mapMemory(memory, 0, VK_WHOLE_SIZE);
    }
};
