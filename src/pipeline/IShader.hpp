#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class IShader {
    public:
        virtual vk::PipelineShaderStageCreateInfo getShaderInfo() const = 0;

        virtual ~IShader() = default;
    };
};
