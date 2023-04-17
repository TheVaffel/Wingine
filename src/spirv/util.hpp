#pragma once

#include <vulkan/vulkan.hpp>

#include "../resource/WrappedDSLCI.hpp"

#include "./ParseResult.hpp"
#include "./Variable.hpp"

#include <map>

namespace wg::spirv::util {
    vk::ShaderStageFlagBits executionModelToShaderStage(ExecutionModel model);
    vk::DescriptorType getDescriptorType(Variable var, ParseResult& parse_result);

    std::map<uint32_t, internal::WrappedDSLCI> getDescriptorSetCreateInfos(const std::span<const std::span<const result::DescriptorSetLayout>>& layouts);
};
