#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class IPipeline {

    public:
        virtual vk::Pipeline getPipeline() const = 0;
        virtual vk::PipelineLayout getPipelineLayout() const = 0;

        virtual ~IPipeline() = default;
    };
};
