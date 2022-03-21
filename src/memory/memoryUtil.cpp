#include "./memoryUtil.hpp"

#include <optional>

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
    };


    /*
     * Memory
     */

    vk::DeviceMemory
    createAndBindMemoryForImage(const vk::Image& image,
                                const vk::Device& device,
                                const vk::PhysicalDeviceMemoryProperties& device_memory_properties) {
        vk::MemoryRequirements requirements;
        requirements = device.getImageMemoryRequirements(image);

        uint32_t memory_type_index = getMemoryTypeIndex(requirements,
                                                        vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                        device_memory_properties);

        vk::DeviceMemory memory = createMemory(requirements.size, memory_type_index, device);

        device.bindImageMemory(image, memory, 0);

        return memory;
    }
};
