#pragma once

#include "./IDrawPass.hpp"

#include "./BasicDrawPassSettings.hpp"

#include "../pipeline.hpp"

namespace wg::internal {


    /*
     * BasicDrawPass - a draw pass that has one color attachment and one
     * depth/stencil attachment in the framebuffer
     */

    class BasicDrawPass : public IDrawPass {

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
        std::shared_ptr<const DeviceManager> device_manager;

        BasicDrawPassSettings settings;

        const Pipeline* pipeline;

        uint32_t num_framebuffers;
        uint32_t current_framebuffer_index;

        SemaphoreSet signal_semaphore_set;
        SemaphoreSet wait_semaphore_set;

        std::vector<Command> commands;

        vk::RenderPass render_pass;

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

        virtual void startRecording(std::shared_ptr<IFramebufferChain> framebufferChain);
        virtual void recordDraw(const std::vector<const Buffer*>& buffers, const IndexBuffer* ind_buf,
                                const std::vector<ResourceSet*>& sets, uint32_t instanceCount = 1);
        virtual void endRecording();

        [[nodiscard]]
        virtual std::shared_ptr<ManagedSemaphoreChain> createAndAddOnFinishSemaphore();

        virtual void resetOnFinishSemaphores(const SemaphoreSet& semaphores);
        virtual void setWaitSemaphores(const SemaphoreSet& semaphoreSet);

        virtual void render();

        virtual ~BasicDrawPass();
    };
};
