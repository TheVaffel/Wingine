#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::renderPassUtil {

    enum class RenderPassType {
        colorDepth,
        depthOnly
    };

    vk::RenderPass createDefaultRenderPass(RenderPassType type,
                                           const vk::Device& device);
};
