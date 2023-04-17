#pragma once

#include <vulkan/vulkan.hpp>

#include "../resource/WrappedDSLCI.hpp"

#include <map>

namespace wg::internal {
    struct PipelineLayoutInfo {
        vk::PipelineLayout layout;
        std::map<uint32_t, WrappedDSLCI> set_layout_info_map;
        std::map<uint32_t, vk::DescriptorSetLayout> set_layout_map;
    };
};
