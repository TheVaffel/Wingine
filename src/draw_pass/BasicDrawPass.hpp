#pragma once

#include "./DrawPassBase.hpp"

#include "./BasicDrawPassSettings.hpp"

#include "../pipeline.hpp"

namespace wg::internal {

    /*
     * BasicDrawPass - a draw pass that has one color attachment and one
     * depth/stencil attachment in the framebuffer
     */

    class BasicDrawPass : public DrawPassBase {

        /*
         * Utility struct to hold stage flags that must be persisted on call stack
         * when constructing submit info
         */
        struct SubmitInfoData {
            vk::SubmitInfo submit_info;
            std::vector<vk::PipelineStageFlags> wait_stage_flags;
        };

        std::shared_ptr<const CommandManager> command_manager;
        std::shared_ptr<const QueueManager> queue_manager;

        BasicDrawPassSettings settings;

        const Pipeline* pipeline;

        uint32_t num_framebuffers;
        uint32_t current_framebuffer_index;

        vk::RenderPass render_pass;
        CommandChainController command_chain;

        bool is_recording;

        void startRecordCommandBuffer(const IFramebuffer& framebuffer, uint32_t index);

        void createSubmitInfo(SubmitInfoData& construction_data);
        std::vector<vk::PipelineStageFlags> getPipelineWaitStageFlags();
        void applyWaitSemaphoresToSubmitInfo(SubmitInfoData& construction_data);
        void applySignalSemaphoresToSubmitInfo(SubmitInfoData& construction_data);

    public:
        BasicDrawPass(const Pipeline* pipeline,
                      uint32_t num_framebuffers,
                      const BasicDrawPassSettings& settings,
                      std::shared_ptr<const CommandManager> command_manager,
                      std::shared_ptr<const QueueManager> queue_manager,
                      std::shared_ptr<const DeviceManager> device_manager);

        virtual CommandChainController& getCommandChain() override;

        virtual void render() override;

        virtual ~BasicDrawPass();
    };
};
