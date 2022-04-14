#include "./CommandManager.hpp"

#include "./log.hpp"

namespace wg::internal {

    CommandManager::CommandManager(std::shared_ptr<const DeviceManager> device_manager,
                                   std::shared_ptr<const QueueManager> queue_manager)
        : device_manager(device_manager), queue_manager(queue_manager) {

        this->initCommandPools();

        this->initCommandBuffers();
        this->initCommandFences();
        this->resetCommandBuffers();
    }

    const Command& CommandManager::getPresentCommand() const {
        if (!this->queue_manager->hasPresentQueue()) {
            throw std::runtime_error("[CommandManager] Does not have a present command");
        }
        return this->present_command;
    }

    const Command& CommandManager::getComputeCommand() const {
        return this->compute_command;
    }

    const Command& CommandManager::getGeneralCommand() const {
        return this->general_command;
    }

    const vk::CommandPool CommandManager::getGraphicsCommandPool() const {
        return this->graphics_command_pool;
    }

    void CommandManager::initCommandPools() {
        vk::CommandPoolCreateInfo cpi;
        cpi.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cpi.setQueueFamilyIndex(this->queue_manager->getGraphicsQueueIndex());
        this->graphics_command_pool = this->device_manager->getDevice().createCommandPool(cpi);

        if (this->queue_manager->hasPresentQueue()) {
            cpi.setQueueFamilyIndex(this->queue_manager->getPresentQueueIndex());
            this->present_command_pool = this->device_manager->getDevice().createCommandPool(cpi);
        }

        if (this->queue_manager->hasComputeQueue()) {
            cpi.setQueueFamilyIndex(this->queue_manager->getComputeQueueIndex());
            this->compute_command_pool = this->device_manager->getDevice().createCommandPool(cpi);
        }
    }

    void CommandManager::initCommandBuffers() {
        vk::CommandBufferAllocateInfo cbi;
        cbi.setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        cbi.setCommandPool(this->graphics_command_pool);
        this->general_command.buffer = this->device_manager->getDevice().allocateCommandBuffers(cbi)[0];

        if (this->queue_manager->hasPresentQueue()) {
            cbi.setCommandPool(this->present_command_pool);
            this->present_command.buffer = this->device_manager->getDevice().allocateCommandBuffers(cbi)[0];
        }

        if (this->queue_manager->hasComputeQueue()) {
            cbi.setCommandPool(this->compute_command_pool);
            this->compute_command.buffer = this->device_manager->getDevice().allocateCommandBuffers(cbi)[0];
        }
    }

    void CommandManager::initCommandFences() {
        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->general_command.fence =
            this->device_manager->getDevice().createFence(fci);

        if (this->queue_manager->hasPresentQueue()) {
            this->present_command.fence =
                this->device_manager->getDevice().createFence(fci);
        }

        if (this->queue_manager->hasComputeQueue()) {
            this->compute_command.fence =
                this->device_manager->getDevice().createFence(fci);
        }
    }

    void CommandManager::resetCommandBuffers() {

        this->general_command.buffer
            .reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        if (this->queue_manager->hasPresentQueue()) {
            this->present_command.buffer
                .reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        }

        if(this->queue_manager->hasComputeQueue()) {

            this->compute_command.buffer
                .reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        }
    }


    std::vector<Command> CommandManager::createGraphicsCommands(uint32_t num_commands) const {
        std::vector<Command> commands(num_commands);
        vk::CommandBufferAllocateInfo cbi;
        cbi.setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1)
            .setCommandPool(this->graphics_command_pool);

        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (uint32_t i = 0; i < num_commands; i++) {
            commands[i].buffer = this->device_manager->getDevice().allocateCommandBuffers(cbi)[0];
            commands[i].fence = this->device_manager->getDevice().createFence(fci);
        }

        return commands;
    }

    void CommandManager::destroyGraphicsCommands(const std::vector<Command>& commands) const {
        const vk::Device& device = this->device_manager->getDevice();
        for (uint32_t i = 0; i < commands.size(); i++) {
            device.freeCommandBuffers(this->graphics_command_pool,
                                      1, &commands[i].buffer);

            _wassert_result(device.waitForFences(1, &commands[i].fence, true, UINT64_MAX),
                            "wait for deleting graphics command fence finish");
            device.destroyFence(commands[i].fence);
        }
    }


    CommandManager::~CommandManager() {
        const vk::Device& device = this->device_manager->getDevice();

        device.freeCommandBuffers(this->graphics_command_pool,
                                        1, &this->general_command.buffer);

        device.destroyCommandPool(this->graphics_command_pool);

        _wassert_result(device.waitForFences(1, &this->general_command.fence, true, UINT64_MAX),
                        "wait for general purpose command finish");
        device.destroy(this->general_command.fence, nullptr);


        if (this->queue_manager->hasPresentQueue()) {
            device.freeCommandBuffers(this->present_command_pool,
                                      1, &this->present_command.buffer);

            device.destroyCommandPool(this->present_command_pool);

            _wassert_result(device.waitForFences(1, &this->present_command.fence, true, UINT64_MAX),
                            "wait for present command finish");
            device.destroyFence(this->present_command.fence);
        }

        if(this->queue_manager->hasComputeQueue()) {
            device.freeCommandBuffers(this->compute_command_pool,
                                      1, &this->compute_command.buffer);

            device.destroyCommandPool(this->compute_command_pool);
            _wassert_result(device.waitForFences(1, &this->compute_command.fence, true, UINT64_MAX),
                            "wait for compute command finish");
            device.destroyFence(this->compute_command.fence);
        }
    }
};
