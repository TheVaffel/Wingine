#pragma once

#include "../CommandManager.hpp"

#include "../framebuffer/FramebufferTextureChain.hpp"
#include "../framebuffer/IFramebufferChain.hpp"
#include "../resource/IResourceSetChain.hpp"

#include "../buffer.hpp"
#include "../pipeline.hpp"

#include "../util/IndexCounter.hpp"

#include "./CommandControllerSettings.hpp"

#include "../sync/EventChain.hpp"

namespace wg::internal {
    class CommandChainController {
        std::vector<Command> commands;

        IndexCounter command_index;

        std::shared_ptr<const CommandManager> command_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        // For sanity checking indices
        std::vector<std::shared_ptr<IResourceSetChain>> recorded_resource_sets;

        vk::RenderPass render_pass;
        const Pipeline* pipeline;

        CommandControllerSettings settings;

        bool is_recording;
        bool is_recording_render_pass;

        std::shared_ptr<IFramebufferChain> current_framebuffer_chain;

        void beginRenderPass();
        void endRenderPass();

    public:

        CommandChainController(uint32_t num_commands,
                               const CommandControllerSettings& settings,
                               const vk::RenderPass& render_pass,
                               const Pipeline* pipeline,
                               std::shared_ptr<const CommandManager> command_manager,
                               std::shared_ptr<const DeviceManager> device_manager);

        void startRecording(std::shared_ptr<IFramebufferChain> framebuffer_chain);

        void recordMakeFramebufferIntoTexture(std::shared_ptr<FramebufferTextureChain> framebuffer_texture);
        void recordMakeTextureIntoFramebuffer(std::shared_ptr<FramebufferTextureChain> framebuffer_texture);
        void recordDraw(const std::vector<const Buffer*>& vertex_buffers,
                        const IndexBuffer* ind_buf,
                        const std::vector<std::shared_ptr<IResourceSetChain>>& resource_sets,
                        uint32_t instanceCount = 1);


        /* NB: Event synchronization is not generalized properly, usage is discouraged before cleanup */
        void recordSetEvent(std::shared_ptr<EventChain> event);
        void recordWaitEvent(std::shared_ptr<EventChain> event, std::shared_ptr<IFramebufferChain> framebuffer_chain);
        void recordResetEvent(std::shared_ptr<EventChain> event);

        void endRecording();

        void sanityCheckRecordedResourceSets();

        Command& getCurrentCommand();
        void swapCommands();

        ~CommandChainController();
    };
};
