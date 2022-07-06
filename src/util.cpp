#include "./util.hpp"

#include <iostream>

#include "./util/log.hpp"

namespace wg {

    /*
     * Utility functions
     */

    uint32_t _get_memory_type_index(uint32_t type_bits,
                                    vk::MemoryPropertyFlags requirements_mask,
                                    const vk::PhysicalDeviceMemoryProperties& mem_props) {
        for(uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((type_bits & (1 << i))) {
                if ((mem_props.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                    return i;
                }
            }
        }

        _wlog_error("Could not find appropriate memory type");
        exit(-1);
    }
};
