#include "./BasicComputePipeline.hpp"

#include "./pipelineUtil.hpp"
#include "./WrappedPLCI.hpp"

#include "../core/constants.hpp"

#include "../resource/IResourceSetChain.hpp"
#include "../resource/descriptorUtil.hpp"
#include "../resource/ResourceBinding.hpp"
#include "../resource/BasicResourceSetChain.hpp"

#include "../sync/fenceUtil.hpp"

#include "../spirv/util.hpp"

#include "../util/log.hpp"

namespace wg::internal {

    BasicComputePipeline::BasicComputePipeline(
        const std::shared_ptr<IShader>& shader,
        const vk::DescriptorPool& descriptor_pool,
        std::shared_ptr<DeviceManager> device_manager,
        std::shared_ptr<CommandManager> command_manager,
        std::shared_ptr<QueueManager> queue_manager,
        const vk::PipelineCache& pipeline_cache)
        : SynchronizedQueueOperationBase(constants::preferred_swapchain_image_count,
                                         device_manager),
          device_manager(device_manager),
          command_manager(command_manager) {

        this->descriptor_pool = descriptor_pool;

        this->command = command_manager->createGraphicsCommands(1)[0];

        vk::Device device = device_manager->getDevice();

        auto raw_layouts = shader->getLayouts();
        for (auto& raw_layout : raw_layouts) {
            this->layout_info.set_layout_map[raw_layout.set_binding] = descriptorUtil::createDescriptorSetLayoutFromBindings(raw_layout.bindings, device);
        }

        WrappedPLCI layoutCreateInfo = pipelineUtil::createLayoutInfo(this->layout_info.set_layout_map);

        this->layout_info.layout = device.createPipelineLayout(layoutCreateInfo.getCreateInfo());

        vk::ComputePipelineCreateInfo cpci;
        cpci.setLayout(this->layout_info.layout)
            .setStage(shader->getShaderInfo());

        this->pipeline = device.createComputePipeline(pipeline_cache,
                                                      {cpci}).value;

        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->compute_queue = queue_manager->getComputeQueue();
    }

    void BasicComputePipeline::execute(const std::vector<std::vector<ResourceBinding>>& resources,
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
            // Create resource set for every execution, not optimized
            std::shared_ptr<IResourceSetChain> resource_set =
                std::make_shared<BasicResourceSetChain>(resources[i][0].resource->getElementChainLength(),
                                                        resources[i],
                                                        this->layout_info.set_layout_map[i],
                                                        this->descriptor_pool,
                                                        this->device_manager);

            sets[i] = resource_set->getCurrentResourceSet().getDescriptorSet();
        }

        if (sets.size()) {
            this->command.buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                    this->layout_info.layout,
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
            .setPCommandBuffers(&this->command.buffer)
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
        this->device_manager->getDevice().destroyPipelineLayout(this->layout_info.layout);
        this->command_manager->destroyGraphicsCommands({this->command});
        for (auto& set_and_layout : this->layout_info.set_layout_map) {
            this->device_manager->getDevice().destroy(set_and_layout.second);
        }
    }
};
