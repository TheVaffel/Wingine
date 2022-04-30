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

    vk::Format _get_format(wg::ComponentType type, int num_components) {
        _wassert(num_components > 0 && num_components <= 4,
                 "Number of components in vertex attribute can be at most 4");
        // At least for now, formats are a bit strict

        // VK_FORMAT_R32_SFLOAT = 100 etc.. are defined in vulkan_core.h

        switch(type) {
        case wg::tFloat32:
            return vk::Format(97 + num_components * 3);
        case wg::tFloat64:
            return vk::Format(109 + num_components * 3);
        case wg::tInt32:
            return vk::Format(96 + num_components * 3);
        case wg::tInt64:
            return vk::Format(95 + num_components * 3);
        }

        return vk::Format::eR32Sfloat;
    }

};
