#include "./compute_pipeline.hpp"

#include "./Wingine.hpp"

namespace wg {

    ComputePipeline::ComputePipeline(Wingine& wing,
                                     const std::vector<vk::DescriptorSetLayout>& resourceSetLayouts,
                                     Shader* shader) {
        vk::Device device = wing.getDevice();

        std::vector<vk::DescriptorSetLayout> layouts(resourceSetLayouts.size());
        for(unsigned int i = 0; i < resourceSetLayouts.size(); i++) {
            layouts[i] = resourceSetLayouts[i];
        }

        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr)
            .setSetLayoutCount(layouts.size())
            .setPSetLayouts(layouts.data());

        this->layout = device.createPipelineLayout(layoutCreateInfo);

        vk::ComputePipelineCreateInfo cpci;
        cpci.setLayout(this->layout)
            .setStage(shader->shader_info);

        this->pipeline = device.createComputePipeline(wing.pipeline_cache,
                                                      {cpci}).value;

        vk::CommandBufferAllocateInfo cbi;
        cbi.setCommandPool(wing.getGraphicsCommandPool())
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
        command.buffer = device.allocateCommandBuffers(cbi)[0];

        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

        command.fence = device.createFence(fci);
    }

};
