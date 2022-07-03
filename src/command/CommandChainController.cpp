#include "./CommandChainController.hpp"

#include "./recordUtil.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {

    CommandChainController::CommandChainController(uint32_t num_commands,
                                                   const CommandControllerSettings& settings,
                                                   const vk::RenderPass& render_pass,
                                                   const Pipeline* pipeline,
                                                   std::shared_ptr<const CommandManager> command_manager,

                                                   std::shared_ptr<const DeviceManager> device_manager)
    : command_index(num_commands),
      command_manager(command_manager),
      device_manager(device_manager),
      render_pass(render_pass),
      pipeline(pipeline),
      is_recording(false),
      is_recording_render_pass(false) {
        this->commands = command_manager->createGraphicsCommands(num_commands);
        this->settings = settings;
    }

    void CommandChainController::startRecording(std::shared_ptr<IFramebufferChain> framebuffer_chain) {
        fl_assert_eq(this->is_recording, false);

        fl_assert_eq(this->commands.size(), framebuffer_chain->getNumFramebuffers());
        for (uint32_t i = 0; i < this->commands.size(); i++) {
            recordUtil::beginRecording(this->commands[i].buffer);
        }

        this->current_framebuffer_chain = framebuffer_chain;
        this->recorded_resource_sets.clear();

        this->is_recording = true;
    }

    void CommandChainController::beginRenderPass() {
        fl_assert_eq(this->is_recording, true);
        fl_assert_eq(this->is_recording_render_pass, false);

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            recordUtil::beginRenderPass(this->settings,
                                        this->commands[i].buffer,
                                        this->render_pass,
                                        this->current_framebuffer_chain->getFramebuffer(i),
                                        this->pipeline);
        }

        this->is_recording_render_pass = true;
    }

    void CommandChainController::endRenderPass() {
        fl_assert_eq(this->is_recording_render_pass, true);

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            recordUtil::endRenderPass(this->commands[i].buffer);
        }

        this->is_recording_render_pass = false;
    }

    void CommandChainController::recordMakeFramebufferIntoTexture(std::shared_ptr<FramebufferTextureChain> framebuffer_texture) {
        fl_assert_eq(this->is_recording, true);

        if (this->is_recording_render_pass) {
            this->endRenderPass();
        }

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            const IFramebuffer& framebuffer = framebuffer_texture->getFramebuffer(i);
            recordUtil::recordMakeFramebufferIntoTexture(
                                                         this->commands[i].buffer,
                                                         framebuffer.hasColorImage() ? framebuffer.getColorImage() : framebuffer.getDepthImage(),
                                                         this->device_manager->getDevice());
        }
    }

    void CommandChainController::recordMakeTextureIntoFramebuffer(std::shared_ptr<FramebufferTextureChain> framebuffer_texture) {
        fl_assert_eq(this->is_recording, true);

        if (this->is_recording_render_pass) {
            this->endRenderPass();
        }

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            const IFramebuffer& framebuffer = framebuffer_texture->getFramebuffer(i);
            recordUtil::recordMakeTextureIntoFramebufferAndClear(this->commands[i].buffer,
                                                                 framebuffer.hasColorImage() ? framebuffer.getColorImage() : framebuffer.getDepthImage(),
                                                                 this->device_manager->getDevice());

        }

    }

    void CommandChainController::recordDraw(const std::vector<const Buffer*>& vertex_buffers,
                                            const IndexBuffer* ind_buf,
                                            const std::vector<std::shared_ptr<IResourceSetChain>>& sets,
                                            uint32_t instance_count) {
        fl_assert_eq(this->is_recording, true);

        if (!is_recording_render_pass) {
            this->beginRenderPass();
        }

        for (uint32_t i = 0; i < sets.size(); i++) {
            this->recorded_resource_sets.push_back(sets[i]);
        }

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            recordUtil::recordDrawForCommand(this->commands[i].buffer,
                                             this->pipeline,
                                             vertex_buffers,
                                             ind_buf,
                                             sets,
                                             i,
                                             instance_count);
        }
    }

    /*
     * NB: Event synchronization is not generalized properly, usage is discouraged before cleanup
     */

    void CommandChainController::recordSetEvent(std::shared_ptr<EventChain> event) {
        fl_assert_eq(this->is_recording, true);

        if (this->is_recording_render_pass) {
            this->endRenderPass();
        }

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            this->commands[i].buffer.setEvent(event->getEventAt(i),
               vk::PipelineStageFlagBits::eBottomOfPipe);
        }
    }

    void CommandChainController::recordWaitEvent(std::shared_ptr<EventChain> event,
                                                 std::shared_ptr<IFramebufferChain> framebuffer_chain) {
        fl_assert_eq(this->is_recording, true);
        if (!this->is_recording_render_pass) {
            this->beginRenderPass();
        }

        vk::MemoryBarrier memory_barrier;
        memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead);


        for (uint32_t i = 0; i < this->commands.size(); i++) {

            const IImage& image = framebuffer_chain->getFramebuffer(i).getDepthImage();

            vk::ImageMemoryBarrier image_memory_barrier;
            vk::ImageSubresourceRange range;
            range.setAspectMask(image.getDefaultAspect())
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
                .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcQueueFamilyIndex(0) // Fix
                .setDstQueueFamilyIndex(0) // Fix
                .setImage(framebuffer_chain->getFramebuffer(i).getDepthImage().getImage()) // Depth -> general
                .setSubresourceRange(range);

            this->commands[i].buffer.waitEvents(event->getEventAt(i),
                                                vk::PipelineStageFlagBits::eBottomOfPipe,
                                                vk::PipelineStageFlagBits::eTopOfPipe,
                                                { /* memory_barrier */ }, {}, { image_memory_barrier });
        }
    }

    void CommandChainController::recordResetEvent(std::shared_ptr<EventChain> event) {
        fl_assert_eq(this->is_recording, true);
        if (this->is_recording_render_pass) {
            this->endRenderPass();
        }

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            this->commands[i].buffer.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, {}, {}, {}, {});
            this->commands[i].buffer.resetEvent(event->getEventAt(i), vk::PipelineStageFlagBits::eBottomOfPipe);
        }

    }

    void CommandChainController::endRecording() {
        fl_assert_eq(this->is_recording, true);
        if (this->is_recording_render_pass) {
            this->endRenderPass();
        }

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            recordUtil::endRecording(this->commands[i].buffer);
        }

        this->is_recording = false;
    }

    void CommandChainController::sanityCheckRecordedResourceSets() {
        for (uint32_t i = 0; i < this->recorded_resource_sets.size(); i++) {
            fl_assert_eq(this->recorded_resource_sets[i]->getCurrentResourceIndex(),
                         this->command_index.getCurrentIndex());
        }
    }

    Command& CommandChainController::getCurrentCommand() {
        return this->commands[this->command_index.getCurrentIndex()];
    }

    void CommandChainController::swapCommands() {
        this->command_index.incrementIndex();
    }

    CommandChainController::~CommandChainController() {
        this->command_manager->destroyGraphicsCommands(this->commands);
    }
};
