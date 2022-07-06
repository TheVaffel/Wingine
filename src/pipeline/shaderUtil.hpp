#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "./ShaderStage.hpp"

namespace wg::internal::shaderUtil {
    vk::ShaderStageFlagBits getShaderStageBit(ShaderStage stage);
};
