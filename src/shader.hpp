#pragma once

#include <vulkan/vulkan.hpp>

namespace wg {

    class Shader {
        vk::PipelineShaderStageCreateInfo shader_info;

        Shader(vk::Device& device,
               uint64_t stage,
               std::vector<uint32_t>& spirv);

        friend class Pipeline;
        friend class Wingine;
        friend class ComputePipeline;
    };
};
