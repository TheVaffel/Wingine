#ifndef WG_UTIL_HPP
#define WG_UTIL_HPP

#include "declarations.hpp"

#include <string>

#include <vulkan/vulkan.hpp>

namespace wg {

    /*
     * Utility functions
     */

    uint32_t _get_memory_type_index(uint32_t type_bits,
                                    vk::MemoryPropertyFlags requirements_mask,
                                    const vk::PhysicalDeviceMemoryProperties& mem_props);
};

#endif // WG_UTIL_HPP
