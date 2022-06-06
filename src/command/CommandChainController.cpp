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
      is_recording(false) {
        this->commands = command_manager->createGraphicsCommands(num_commands);
        this->settings = settings;
    }

    void CommandChainController::startRecording(std::shared_ptr<IFramebufferChain> framebuffer_chain) {
        fl_assert_eq(this->is_recording, false);

        fl_assert_eq(this->commands.size(), framebuffer_chain->getNumFramebuffers());
        for (uint32_t i = 0; i < this->commands.size(); i++) {
            recordUtil::beginGraphicsCommand(this->settings,
                                             this->commands[i].buffer,
                                             this->render_pass,
                                             framebuffer_chain->getFramebuffer(i),
                                             this->pipeline);

        }

        this->recorded_resource_sets.clear();

        this->is_recording = true;
    }

    void CommandChainController::recordMakeFramebufferIntoTexture(std::shared_ptr<FramebufferTextureChain> framebuffer_texture) {
        fl_assert_eq(this->is_recording, true);
    }

    void CommandChainController::recordMakeTextureIntoFramebuffer(std::shared_ptr<FramebufferTextureChain> framebuffer_texture) {
        fl_assert_eq(this->is_recording, true);

    }

    void CommandChainController::recordDraw(const std::vector<const Buffer*>& vertex_buffers,
                                            const IndexBuffer* ind_buf,
                                            const std::vector<std::shared_ptr<IResourceSetChain>>& sets,
                                            uint32_t instance_count) {
        fl_assert_eq(this->is_recording, true);

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

    void CommandChainController::endRecording() {
        fl_assert_eq(this->is_recording, true);

        for (uint32_t i = 0; i < this->commands.size(); i++) {
            this->commands[i].buffer.endRenderPass();
            this->commands[i].buffer.end();
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
        // this->device_manager->getDevice().destroy(this->render_pass);
    }
};
