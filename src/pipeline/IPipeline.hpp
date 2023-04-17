#pragma once

#include <vulkan/vulkan.hpp>

#include "./PipelineLayoutInfo.hpp"

namespace wg::internal {
    class IPipeline {

    public:
        virtual vk::Pipeline getPipeline() const = 0;
        virtual const PipelineLayoutInfo& getPipelineInfo() const = 0;

        virtual ~IPipeline() = default;
    };
};
