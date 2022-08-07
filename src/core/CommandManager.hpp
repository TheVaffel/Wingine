#pragma once

#include <vulkan/vulkan.hpp>

#include "./DeviceManager.hpp"
#include "./QueueManager.hpp"

namespace wg::internal {

    struct Command {
        vk::CommandBuffer buffer;
        vk::Fence fence;
    };

    class CommandManager {

        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const QueueManager> queue_manager;


        vk::CommandPool present_command_pool;
        vk::CommandPool graphics_command_pool;
        vk::CommandPool compute_command_pool;

        Command present_command;
        Command compute_command;
        Command general_command;

        void initCommandPools();
        void initCommandBuffers();
        void initCommandFences();

        void resetCommandBuffers();

    public:

        CommandManager(std::shared_ptr<const DeviceManager> device_manager,
                       std::shared_ptr<const QueueManager> queue_manager);
        ~CommandManager();

        CommandManager(const CommandManager&) = delete;

        const Command& getPresentCommand() const;
        const Command& getComputeCommand() const;
        const Command& getGeneralCommand() const;

        std::vector<Command> createGraphicsCommands(uint32_t num_commands) const;
        void destroyGraphicsCommands(const std::vector<Command>& commands) const;

        const vk::CommandPool getGraphicsCommandPool() const;
    };
};
