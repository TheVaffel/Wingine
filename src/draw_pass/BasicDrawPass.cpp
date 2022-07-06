#include "./BasicDrawPass.hpp"

#include "../util/log.hpp"

#include "../sync/fenceUtil.hpp"

#include <flawed_assert.hpp>

#include "../command/CommandControllerSettings.hpp"

namespace wg::internal {

    namespace {
        CommandControllerSettings commandSettingsFromRenderPassSettings(const CommandRenderPassSettings& render_pass_setup) {
            return { render_pass_setup } ;
        }

        vk::RenderPass createRenderPass(const BasicDrawPassSettings& settings,
                                        const vk::Device& device) {

        renderPassUtil::RenderPassSetup render_pass_setup;
        render_pass_setup.setColorClears({ settings.render_pass_settings.getShouldClearColor() });
        render_pass_setup.setDepthClear(settings.render_pass_settings.getShouldClearDepth());
        render_pass_setup.setNumColorAttachments(settings.render_pass_settings.getNumColorAttachments());
        render_pass_setup.setFinalizeAsTexture(settings.render_pass_settings.getFinalizeAsTexture());

        return renderPassUtil::createRenderPass(render_pass_setup, device);

        }
    };

    BasicDrawPass::BasicDrawPass(std::shared_ptr<IPipeline> pipeline,
                                 uint32_t num_framebuffers,
                                 const BasicDrawPassSettings& settings,
                                 std::shared_ptr<const CommandManager> command_manager,
                                 std::shared_ptr<const QueueManager> queue_manager,
                                 std::shared_ptr<const DeviceManager> device_manager)
        : DrawPassBase(num_framebuffers, device_manager),
          command_manager(command_manager),
          queue_manager(queue_manager),
          settings(settings),
          pipeline(pipeline),
          num_framebuffers(num_framebuffers),
          current_framebuffer_index(0),
          render_pass(createRenderPass(settings, device_manager->getDevice())),
          command_chain(num_framebuffers,
                        commandSettingsFromRenderPassSettings(settings.render_pass_settings),
                        render_pass,
                        pipeline,
                        command_manager,
                        device_manager),
          is_recording(false) {
    }

    std::vector<vk::PipelineStageFlags> BasicDrawPass::getPipelineWaitStageFlags() {
        // Bigbig optimization opportunity
        return std::vector<vk::PipelineStageFlags>(this->getWaitSemaphores().getNumSemaphores(),
                                                   vk::PipelineStageFlagBits::eTopOfPipe);
    }

    void BasicDrawPass::applyWaitSemaphoresToSubmitInfo(SubmitInfoData& construction_data) {
        construction_data.wait_stage_flags = this->getPipelineWaitStageFlags();
        construction_data.submit_info
            .setPWaitDstStageMask(construction_data.wait_stage_flags.data())
            .setWaitSemaphores(this->getWaitSemaphores().getCurrentRawSemaphores())
            .setWaitSemaphoreCount(this->getWaitSemaphores().getNumSemaphores());

        this->getWaitSemaphores().swapSemaphores();
    }

    void BasicDrawPass::applySignalSemaphoresToSubmitInfo(SubmitInfoData& construction_data) {
        construction_data.submit_info
            .setSignalSemaphores(this->getSignalSemaphores().getCurrentRawSemaphores());

        this->getSignalSemaphores().swapSemaphores();
    }

    void BasicDrawPass::createSubmitInfo(SubmitInfoData& construction_data) {
        construction_data.submit_info.setCommandBufferCount(1)
            .setPCommandBuffers(&this->command_chain.getCurrentCommand().buffer);

        this->applyWaitSemaphoresToSubmitInfo(construction_data);
        this->applySignalSemaphoresToSubmitInfo(construction_data);
    }

    void BasicDrawPass::awaitCurrentCommand() {
        fenceUtil::awaitFence(this->command_chain.getCurrentCommand().fence,
                              this->device_manager->getDevice());
    }

    void BasicDrawPass::render() {
        this->command_chain.sanityCheckRecordedResourceSets();

        SubmitInfoData submit_info_data;
        this->createSubmitInfo(submit_info_data);

        this->awaitCurrentCommand();
        fenceUtil::resetFence(this->command_chain.getCurrentCommand().fence,
                              this->device_manager->getDevice());

        _wassert_result(this->queue_manager->getGraphicsQueue()
                        .submit(1,
                                &submit_info_data.submit_info,
                                this->command_chain.getCurrentCommand().fence),
                        "submitting draw pass command to queue");

        this->command_chain.swapCommands();
    }

    CommandChainController& BasicDrawPass::getCommandChain() {
        return this->command_chain;
    }

    BasicDrawPass::~BasicDrawPass() {
        this->device_manager->getDevice().destroy(this->render_pass);
    }
};
