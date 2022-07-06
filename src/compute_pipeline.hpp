#pragma once

#include "./declarations.hpp"
#include "./shader.hpp"
#include "./CommandManager.hpp"

namespace wg {

    class ComputePipeline {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;

        internal::Command command;

        ComputePipeline(Wingine& wing,
                        const std::vector<vk::DescriptorSetLayout>& resourceSetLayout,
                        Shader* shaders);

        friend class ComputeFamily;
        friend class Wingine;
    };
};
