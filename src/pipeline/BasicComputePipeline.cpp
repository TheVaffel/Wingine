#include "./BasicComputePipeline.hpp"
#include "../constants.hpp"

#include "../resource/IResourceSetChain.hpp"

#include "../sync/fenceUtil.hpp"

#include "../util/log.hpp"

namespace wg::internal {

    BasicComputePipeline::BasicComputePipeline(
        const std::vector<vk::DescriptorSetLayout>& resource_set_layouts,
        const std::shared_ptr<IShader>& shader,
        std::shared_ptr<DeviceManager> device_manager,
        std::shared_ptr<CommandManager> command_manager,
        std::shared_ptr<QueueManager> queue_manager,
        const vk::PipelineCache& pipeline_cache)
        : SynchronizedQueueOperationBase(constants::preferred_swapchain_image_count,
                                         device_manager),
          device_manager(device_manager),
          command_manager(command_manager) {

        this->command = command_manager->createGraphicsCommands(1)[0];

        vk::Device device = device_manager->getDevice();

        std::vector<vk::DescriptorSetLayout> layouts(resource_set_layouts);

        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr)
            .setSetLayouts(layouts);

        this->layout = device.createPipelineLayout(layoutCreateInfo);

        vk::ComputePipelineCreateInfo cpci;
        cpci.setLayout(this->layout)
            .setStage(shader->getShaderInfo());

        this->pipeline = device.createComputePipeline(pipeline_cache,
                                                      {cpci}).value;

        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->command.fence = device.createFence(fci);

        this->compute_queue = queue_manager->getComputeQueue();
    }

    void BasicComputePipeline::execute(const std::vector<std::shared_ptr<IResourceSetChain>>& resources,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t depth) {
        fenceUtil::awaitAndResetFence(this->command.fence,
                                      this->device_manager->getDevice());

        vk::CommandBufferBeginInfo begin;
        this->command.buffer.begin(begin);
        this->command.buffer.bindPipeline(vk::PipelineBindPoint::eCompute, this->pipeline);

        std::vector<vk::DescriptorSet> sets(resources.size());
        for (uint32_t i = 0; i < sets.size(); i++) {
            sets[i] = resources[i]->getCurrentResourceSet().getDescriptorSet();
        }

        if (sets.size()) {
            this->command.buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                    this->layout,
                                                    0,
                                                    sets.size(),
                                                    sets.data(),
                                                    0,
                                                    nullptr);
        }

        this->command.buffer.dispatch(width, height, depth);
        this->command.buffer.end();

        std::vector<vk::PipelineStageFlags> wait_stages(this->getWaitSemaphores().getNumSemaphores(),
                                                        vk::PipelineStageFlagBits::eTopOfPipe);

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPWaitDstStageMask(wait_stages.data())
            .setWaitSemaphores(this->getWaitSemaphores().getCurrentRawSemaphores())
            .setSignalSemaphores(this->getSignalSemaphores().getCurrentRawSemaphores());

        this->getWaitSemaphores().swapSemaphores();
        this->getSignalSemaphores().swapSemaphores();

        _wassert_result(this->compute_queue
                        .submit(1,
                                &si,
                                this->command.fence));
    }


    void BasicComputePipeline::awaitExecution() {
        fenceUtil::awaitFence(this->command.fence,
                              this->device_manager->getDevice());
    }

    BasicComputePipeline::~BasicComputePipeline() {
        this->device_manager->getDevice().destroyPipeline(this->pipeline);
        this->command_manager->destroyGraphicsCommands({this->command});
    }
};
