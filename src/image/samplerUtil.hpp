#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal::samplerUtil {

    struct BasicSamplerSetup { };

    vk::Sampler createBasicSampler(const BasicSamplerSetup& setup,
                                   const vk::Device& device);

};
