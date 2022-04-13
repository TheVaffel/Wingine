#include "./renderfamily.hpp"

#include "./Wingine.hpp"

#include "./framebuffer/IFramebuffer.hpp"
#include "./log.hpp"

#include <flawed_assert.hpp>

namespace wg {

    namespace {
        std::vector<internal::Command> initializeCommands(uint32_t num_commands,
                                                          const vk::CommandPool& pool,
                                                          const vk::Device& device) {
            std::vector<internal::Command> commands(num_commands);

            vk::CommandBufferAllocateInfo cbi;
            cbi.setCommandPool(pool)
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(1); // Premature optimization... etc.
            for(uint32_t i = 0; i < num_commands; i++) {
                std::vector<vk::CommandBuffer> command_buffers = device.allocateCommandBuffers(cbi);
                commands[i].buffer = command_buffers[0];

                commands[i].buffer
                    .reset(vk::CommandBufferResetFlagBits::eReleaseResources);

                vk::FenceCreateInfo fci;
                fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

                commands[i].fence =
                    device.createFence(fci);
            }

            return commands;
        }

        std::vector<vk::RenderPass>
        initializeRenderPasses(uint32_t num_render_passes,
                               internal::renderPassUtil::RenderPassType render_pass_type,
                               internal::CompatibleRenderPassRegistry& render_pass_registry) {

            std::vector<vk::RenderPass> render_passes(num_render_passes);

            for(uint32_t i = 0; i < num_render_passes; i++) {
                render_passes[i] = render_pass_registry.ensureAndGetRenderPass(render_pass_type);
            }

            return render_passes;
        }
    };

    RenderFamily::RenderFamily(Wingine& wing,
                               std::shared_ptr<internal::CompatibleRenderPassRegistry> renderPassRegistry,
                               const Pipeline* pipeline,
                               bool clear,
                               int num_framebuffers) :
        wing(&wing), pipeline(pipeline), render_pass_registry(renderPassRegistry) {

        if(num_framebuffers == 0) {
            num_framebuffers = wing.getNumFramebuffers();
        }

        this->clears = clear;
        this->num_buffers = num_framebuffers;

        this->render_pass_type = pipeline->render_pass_type;

        this->setFramebufferCount(this->num_buffers);
    }

    void RenderFamily::setFramebufferCount(uint32_t new_count) {
        /*
         * TODO, NB! Release old resources before resizing!
         */

        this->destroyCommandsAndRenderPasses();

        this->num_buffers = new_count;
        this->commands = initializeCommands(this->num_buffers,
                                            this->wing->getGraphicsCommandPool(),
                                            this->wing->getDevice());
        this->render_passes = initializeRenderPasses(this->num_buffers,
                                                     this->render_pass_type,
                                                     *this->render_pass_registry);

    }

    void RenderFamily::destroyCommandsAndRenderPasses() {
        for (const internal::Command& command : this->commands) {
            this->wing->getDevice().destroy(command.fence);
        }

        // Render passes are allocated from CompatibleRenderPassRegistry
    }

    void RenderFamily::startRecording(std::shared_ptr<internal::IFramebufferChain> framebuffer_chain) {
        this->framebuffer_chain = framebuffer_chain;

        if (this->num_buffers != framebuffer_chain->getNumFramebuffers()) {
            this->setFramebufferCount(framebuffer_chain->getNumFramebuffers());
        }

        fl_assert_eq(this->num_buffers, framebuffer_chain->getNumFramebuffers());

        for (uint32_t i = 0; i < this->framebuffer_chain->getNumFramebuffers(); i++) {

            vk::CommandBufferBeginInfo begin;

            vk::Rect2D renderRect;
            renderRect.setOffset({0, 0})
                .setExtent(this->framebuffer_chain->getFramebuffer(i).getDepthImage().getDimensions());

            vk::RenderPassBeginInfo rpb;
            rpb.setRenderPass(this->render_passes[i])
                .setClearValueCount(0)
                .setPClearValues(nullptr)
                .setFramebuffer(this->framebuffer_chain->getFramebuffer(i).getFramebuffer())
                .setRenderArea(renderRect);

            // Size is number of attachments
            std::vector<vk::ClearValue> clear_values;

            if(this->clears) {
                switch (this->render_pass_type) {
                case internal::renderPassUtil::RenderPassType::depthOnly:
                    clear_values.resize(1);
                    clear_values[0].depthStencil.depth = 1.0f;
                    clear_values[0].depthStencil.stencil = 0.0f;
                    break;
                case internal::renderPassUtil::RenderPassType::colorDepth:
                    clear_values.resize(2);
                    clear_values[0].color.setFloat32({0.3f, 0.3f,
                            0.3f, 1.0f});
                    clear_values[1].depthStencil.depth = 1.0f;
                    clear_values[1].depthStencil.stencil = 0.0f;
                }

                rpb.setClearValueCount(clear_values.size())
                    .setPClearValues(clear_values.data());
            }

            vk::Device device = this->wing->getDevice();

            _wassert_result(device.waitForFences(1, &this->commands[i].fence, true, UINT64_MAX),
                            "wait for render family command finish");

            this->commands[i].buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

            this->commands[i].buffer.begin(begin);

            this->commands[i].buffer.beginRenderPass(rpb, vk::SubpassContents::eInline);

            this->commands[i].buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->pipeline->pipeline);
        }
    }

    void RenderFamily::recordDraw(const std::vector<const Buffer*>& vertex_buffers, const IndexBuffer* ind_buf,
                                  const std::vector<ResourceSet*>& sets, int instanceCount){

        for(uint32_t j = 0; j < this->num_buffers; j++) {

            std::vector<vk::DescriptorSet> d_sets(sets.size());
            for(unsigned int i = 0; i < sets.size(); i++) {
                d_sets[i] = sets[i]->descriptor_set;
            }

            if (sets.size()) {
                this->commands[j].buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                            this->pipeline->layout,
                                                            0, d_sets.size(),
                                                            d_sets.data(),
                                                            0, nullptr);
            }

            std::vector<vk::Buffer> vk_buffers(vertex_buffers.size());
            std::vector<vk::DeviceSize> offsets(vertex_buffers.size());

            for(unsigned int i = 0; i < vertex_buffers.size(); i++) {
                vk_buffers[i] = vertex_buffers[i]->buffer;
                offsets[i] = 0;
            }

            this->commands[j].buffer.bindVertexBuffers(0, vk_buffers.size(),
                                                       vk_buffers.data(),
                                                       offsets.data());
            this->commands[j].buffer.bindIndexBuffer(ind_buf->buffer,
                                                     0, vk::IndexType::eUint32);

            this->commands[j].buffer.drawIndexed(ind_buf->num_indices, instanceCount,
                                                 0, 0, 0);
        }
    }

    void RenderFamily::endRecording() {
        for(uint32_t i = 0; i < this->num_buffers; i++) {
            this->commands[i].buffer.endRenderPass();
            this->commands[i].buffer.end();
        }
    }

    void RenderFamily::submit(const std::initializer_list<SemaphoreChain*>& wait_semaphores, int index) {
        uint32_t real_index;
        if (index == -1) {
            real_index = this->current_framebuffer_index;
            this->current_framebuffer_index =
                (this->current_framebuffer_index + 1)
                % this->framebuffer_chain->getNumFramebuffers();
        } else {
            real_index = index;
        }

        _wassert(real_index <= this->num_buffers, "[RenderFamily::submit(int index)] Index too high. This could be because the RenderFamily instance is created with fewer buffers than there are frame buffers, and no index was explicitly set.");

        this->submit_command(wait_semaphores, real_index);
    }

    void RenderFamily::submit_command(const std::initializer_list<SemaphoreChain*>& semaphores, int index) {

        vk::Device device = this->wing->getDevice();
        vk::Queue queue = this->wing->getGraphicsQueue();

        std::vector<vk::PipelineStageFlags> stage_flags(semaphores.size());

        for(unsigned int i = 0; i < semaphores.size(); i++) {
            stage_flags[i] = vk::PipelineStageFlagBits::eTopOfPipe;
        }

        std::vector<vk::Semaphore> wait_sems(semaphores.size());
        std::vector<vk::Semaphore> signal_sems(semaphores.size());
        std::vector<vk::PipelineStageFlags> flags(semaphores.size());
        std::vector<uint64_t> wait_vals(semaphores.size());
        std::vector<uint64_t> signal_vals(semaphores.size());

        int num_wait_sems = SemaphoreChain::getWaitSemaphores(wait_sems.data(), std::begin(semaphores), semaphores.size());
        int num_signal_sems = SemaphoreChain::getSignalSemaphores(signal_sems.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getSemaphoreWaitValues(wait_vals.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getSemaphoreSignalValues(signal_vals.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getWaitStages(flags.data(), std::begin(semaphores), semaphores.size());

        vk::TimelineSemaphoreSubmitInfo tssi;
        tssi.setSignalSemaphoreValueCount(num_signal_sems)
            .setPSignalSemaphoreValues(signal_vals.data())
            .setWaitSemaphoreValueCount(num_wait_sems)
            .setPWaitSemaphoreValues(wait_vals.data());

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&this->commands[index].buffer)
            .setPWaitDstStageMask(stage_flags.data())
            .setSignalSemaphoreCount(num_signal_sems)
            .setPSignalSemaphores(signal_sems.data())
            .setWaitSemaphoreCount(num_wait_sems)
            .setPWaitSemaphores(wait_sems.data())
            .setPNext(&tssi);

        SemaphoreChain::resetModifiers(std::begin(semaphores), semaphores.size());

        // Ensure finished last submission
        _wassert_result(device.waitForFences(1, &this->commands[index].fence, true, (uint64_t)1e9),
                        "wait for last submission in render family");

        _wassert_result(device.resetFences(1, &this->commands[index].fence),
                        "reset fence in render family");

        _wassert_result(queue.submit(1, &si, this->commands[index].fence),
                        "submitting render family command to queue");

    }
};
