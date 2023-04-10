#pragma once

#include <vulkan/vulkan.hpp>

#include "./ParseResult.hpp"
#include "./Variable.hpp"

namespace wg::spirv::util {
    vk::ShaderStageFlagBits executionModelToShaderStage(ExecutionModel model);
    vk::DescriptorType getDescriptorType(Variable var, ParseResult& parse_result);


    vk::DescriptorSetLayout createDescriptorSetLayoutFromBindings(const std::span<const vk::DescriptorSetLayoutBinding>& bindings,
                                                                  const vk::Device& device);
    std::vector<vk::DescriptorSetLayout> mergeDescriptorSetLayouts(const std::span<const std::span<const result::DescriptorSetLayout>>& layouts,
                                                                   const vk::Device& device);
};
