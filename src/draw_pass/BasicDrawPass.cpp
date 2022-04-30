#include "./BasicDrawPass.hpp"

#include "../util/log.hpp"

#include "../sync/fenceUtil.hpp"

#include <iostream>

namespace wg::internal {

    namespace {

        vk::Rect2D getRenderRect(const IFramebuffer& framebuffer) {
            vk::Rect2D renderRect;
            renderRect.setOffset({0, 0})
                .setExtent(framebuffer.getDimensions());

            return renderRect;
        }

        std::vector<vk::ClearValue> getColorAndDepthClearValues(const BasicDrawPassSettings& settings) {
            std::vector<vk::ClearValue> clear_values(2);

            if (settings.shouldClearColor()) {
                clear_values[0].color.setFloat32({ 0.2f, 0.2f, 0.2f, 1.0f });
                clear_values[1].depthStencil.depth = 1.0f;
                clear_values[1].depthStencil.stencil = 0.0f;
            }

            return clear_values;
        }

        struct RenderPassBeginInfoData {
            vk::RenderPassBeginInfo begin_info;
            std::vector<vk::ClearValue> clear_values;
        };

        void createRenderPassBeginInfo(RenderPassBeginInfoData& data,
                                       const BasicDrawPassSettings& settings,
                                       const vk::RenderPass& render_pass,
                                       const IFramebuffer& framebuffer) {

            vk::Rect2D renderRect = getRenderRect(framebuffer);
            data.clear_values = getColorAndDepthClearValues(settings);

            data.begin_info.setRenderPass(render_pass)
                .setFramebuffer(framebuffer.getFramebuffer())
                .setRenderArea(renderRect)
                .setClearValues(data.clear_values);
        }

        void beginGraphicsCommand(const BasicDrawPassSettings& settings,
                                  const vk::CommandBuffer& command_buffer,
                                  const vk::RenderPass& render_pass,
                                  const IFramebuffer& framebuffer,
                                  const Pipeline* pipeline) {

            RenderPassBeginInfoData begin_info_data;
            createRenderPassBeginInfo(begin_info_data,
                                      settings,
                                      render_pass,
                                      framebuffer);

            vk::CommandBufferBeginInfo begin;
            command_buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
            command_buffer.begin(begin);
            command_buffer.beginRenderPass(begin_info_data.begin_info, vk::SubpassContents::eInline);
            command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                        pipeline->getPipeline());
        }

        void recordBindAndDrawVertexBuffers(const vk::CommandBuffer& command_buffer,
                                            const std::vector<const Buffer*>& vertex_buffers,
                                            const IndexBuffer* index_buffer,
                                            uint32_t instance_count) {

            std::vector<vk::Buffer> vk_buffers(vertex_buffers.size());
            std::vector<vk::DeviceSize> offsets(vertex_buffers.size());

            for (uint32_t j = 0; j < vertex_buffers.size(); j++) {
                vk_buffers[j] = vertex_buffers[j]->getBuffer();
                offsets[j] = 0;
            }

            command_buffer.bindVertexBuffers(0, vk_buffers.size(), vk_buffers.data(), offsets.data());

            command_buffer.bindIndexBuffer(index_buffer->getBuffer(), 0, vk::IndexType::eUint32);

            command_buffer.drawIndexed(index_buffer->getNumIndices(), instance_count, 0, 0, 0);
        }

        void recordBindDescriptorSets(const vk::CommandBuffer& command_buffer,
                                      const std::vector<ResourceSet*>& resource_sets,
                                      const Pipeline* pipeline) {

            std::vector<vk::DescriptorSet> descriptor_sets(resource_sets.size());
            for (uint32_t j = 0; j < resource_sets.size(); j++) {
                descriptor_sets[j] = resource_sets[j]->getDescriptorSet();
            }

            if (resource_sets.size()) {
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                  pipeline->getLayout(),
                                                  0,
                                                  descriptor_sets.size(),
                                                  descriptor_sets.data(),
                                                  0,
                                                  nullptr);
            }

        }

        void recordDrawForCommand(const vk::CommandBuffer& command_buffer,
                                  const Pipeline* pipeline,
                                  const std::vector<const Buffer*>& vertex_buffers,
                                  const IndexBuffer* index_buffer,
                                  const std::vector<ResourceSet*>& resource_sets,
                                  uint32_t instance_count) {

            recordBindDescriptorSets(command_buffer, resource_sets, pipeline);

            recordBindAndDrawVertexBuffers(command_buffer,
                                           vertex_buffers,
                                           index_buffer,
                                           instance_count);
        }
    };

    BasicDrawPass::BasicDrawPass(const Pipeline* pipeline,
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
          is_recording(false) {
        this->commands = this->command_manager->createGraphicsCommands(num_framebuffers);

        renderPassUtil::RenderPassSetup render_pass_setup;
        render_pass_setup.setColorClears({ settings.shouldClearColor() });
        render_pass_setup.setDepthClear(settings.shouldClearDepth());

        this->render_pass = renderPassUtil::createRenderPass(render_pass_setup, device_manager->getDevice());
    }

    void BasicDrawPass::startRecordCommandBuffer(const IFramebuffer& framebuffer, uint32_t index) {
        beginGraphicsCommand(this->settings,
                             this->commands[index].buffer,
                             this->render_pass,
                             framebuffer,
                             this->pipeline);
    }

    void BasicDrawPass::startRecording(std::shared_ptr<IFramebufferChain> framebuffer_chain) {

        if (this->is_recording) {
            throw std::runtime_error("[BasicDrawPass] Draw pass is already recording");
        }

        for (uint32_t i = 0; i < this->num_framebuffers; i++) {
            startRecordCommandBuffer(framebuffer_chain->getFramebuffer(i), i);
        }

        this->is_recording = true;
    }

    void BasicDrawPass::recordDraw(const std::vector<const Buffer*>& vertex_buffers,
                                   const IndexBuffer* ind_buf,
                                   const std::vector<ResourceSet*>& input_sets,
                                   uint32_t instance_count) {
        if (!this->is_recording) {
            throw std::runtime_error("[BasicDrawPass] Draw pass is not recording");
        }

        for (uint32_t i = 0; i < this->num_framebuffers; i++) {
            recordDrawForCommand(this->commands[i].buffer,
                                 this->pipeline,
                                 vertex_buffers,
                                 ind_buf,
                                 input_sets,
                                 instance_count);
        }
    }

    void BasicDrawPass::endRecording() {
        if (!this->is_recording) {
            throw std::runtime_error("[BasicDrawPass] Draw pass is not recording");
        }

        for (uint32_t i = 0; i < this->num_framebuffers; i++) {
            this->commands[i].buffer.endRenderPass();
            this->commands[i].buffer.end();
        }

        this->is_recording = false;
    }

    std::vector<vk::PipelineStageFlags> BasicDrawPass::getPipelineWaitStageFlags() {
        // Bigbig optimization opportunity
        return std::vector<vk::PipelineStageFlags>(this->signal_and_wait_semaphores.getWaitSemaphores().getNumSemaphores(),
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
            .setPSignalSemaphores(this->getSignalSemaphores()
                                  .getCurrentRawSemaphores().data())
            .setSignalSemaphoreCount(this->getSignalSemaphores()
                                     .getCurrentRawSemaphores().size());

        this->getSignalSemaphores().swapSemaphores();
    }

    void BasicDrawPass::createSubmitInfo(SubmitInfoData& construction_data) {
        construction_data.submit_info.setCommandBufferCount(1)
            .setPCommandBuffers(&this->commands[this->current_framebuffer_index].buffer);

        this->applyWaitSemaphoresToSubmitInfo(construction_data);
        this->applySignalSemaphoresToSubmitInfo(construction_data);
    }

    void BasicDrawPass::render() {
        SubmitInfoData submit_info_data;
        this->createSubmitInfo(submit_info_data);

        fenceUtil::awaitAndResetFence(this->commands[this->current_framebuffer_index].fence,
                                      this->device_manager->getDevice());

        _wassert_result(this->queue_manager->getGraphicsQueue()
                        .submit(1,
                                &submit_info_data.submit_info,
                                this->commands[this->current_framebuffer_index].fence),
                        "submitting draw pass command to queue");

        this->current_framebuffer_index = (this->current_framebuffer_index + 1) % this->num_framebuffers;
    }

    BasicDrawPass::~BasicDrawPass() {
        this->command_manager->destroyGraphicsCommands(this->commands);
        this->device_manager->getDevice().destroy(this->render_pass);
    }
};
