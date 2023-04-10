#pragma once

#include <vulkan/vulkan.hpp>

#include "../spirv/SpirvModule.hpp"

namespace wg::internal {
    class IShader {
    public:
        virtual vk::PipelineShaderStageCreateInfo getShaderInfo() const = 0;

        virtual const std::span<const spirv::DescriptorSetLayout> getLayouts() const = 0;

        virtual ~IShader() = default;
    };
};
