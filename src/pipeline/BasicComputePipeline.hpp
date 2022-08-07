#pragma once

#include "./IComputePipeline.hpp"
#include "./IShader.hpp"

#include "../DeviceManager.hpp"
#include "../CommandManager.hpp"

#include "../sync/SynchronizedQueueOperationBase.hpp"

namespace wg::internal {
    class BasicComputePipeline : public IComputePipeline, public SynchronizedQueueOperationBase {
        vk::PipelineLayout layout;
        vk::Pipeline pipeline;
        vk::Queue compute_queue;

        Command command;

        std::shared_ptr<DeviceManager> device_manager;
        std::shared_ptr<CommandManager> command_manager;

    public:

        // TODO: Make it use a chain of commands instead of one

        BasicComputePipeline(
            const std::vector<vk::DescriptorSetLayout>& resource_set_layout,
            const std::shared_ptr<IShader>& shader,
            std::shared_ptr<DeviceManager> device_manager,
            std::shared_ptr<CommandManager> command_manager,
            std::shared_ptr<QueueManager> queue_manager,
            const vk::PipelineCache& pipeline_cache);

        ~BasicComputePipeline();

        virtual void execute(const std::vector<std::shared_ptr<IResourceSetChain>>& resources,
                             uint32_t width,
                             uint32_t height = 1,
                             uint32_t depth = 1) final;

        virtual void awaitExecution() final;
    };
};