#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    enum class ShaderStage {
        Vertex = VK_SHADER_STAGE_VERTEX_BIT,
        Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        Compute = VK_SHADER_STAGE_COMPUTE_BIT
    };
};
