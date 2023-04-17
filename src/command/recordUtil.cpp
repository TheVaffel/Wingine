#include "./recordUtil.hpp"

#include "../image/imageUtil.hpp"

namespace wg::internal::recordUtil {

    struct RenderPassBeginInfoData {
        vk::RenderPassBeginInfo begin_info;
        std::vector<vk::ClearValue> clear_values;
    };

    namespace {

        vk::Rect2D getRenderRect(const IFramebuffer& framebuffer) {
            vk::Rect2D renderRect;
            renderRect.setOffset({0, 0})
                .setExtent(framebuffer.getDimensions());

            return renderRect;
        }

        std::vector<vk::ClearValue> getColorAndDepthClearValues(const CommandControllerSettings& settings) {
            std::vector<vk::ClearValue> clear_values;

            // Color
            for (uint32_t i = 0; i < settings.command_render_pass_settings.getNumColorAttachments(); i++) {
                vk::ClearValue clear_value;
                clear_value.color.setFloat32({ 0.2f, 0.2f, 0.2f, 1.0f });
                clear_values.push_back(clear_value);
            }

            // Depth
            vk::ClearValue clear_value;
            clear_value.depthStencil.depth = 1.0f;
            clear_value.depthStencil.stencil = 0.0f;
            clear_values.push_back(clear_value);

            return clear_values;
        }

        void createRenderPassBeginInfo(RenderPassBeginInfoData& data,
                                       const CommandControllerSettings& settings,
                                       const vk::RenderPass& render_pass,
                                       const IFramebuffer& framebuffer) {

            vk::Rect2D renderRect = getRenderRect(framebuffer);
            data.clear_values = getColorAndDepthClearValues(settings);

            data.begin_info.setRenderPass(render_pass)
                .setFramebuffer(framebuffer.getFramebuffer())
                .setRenderArea(renderRect)
                .setClearValues(data.clear_values);
        }


        void recordBindAndDrawVertexBuffers(const vk::CommandBuffer& command_buffer,
                                            const std::vector<std::shared_ptr<IBuffer>>& vertex_buffers,
                                            const std::shared_ptr<IIndexBuffer> index_buffer,
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
    };

    void beginRecording(const vk::CommandBuffer& command_buffer) {
        vk::CommandBufferBeginInfo begin;
        command_buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        command_buffer.begin(begin);

    }

    void beginRenderPass(const CommandControllerSettings& settings,
                         const vk::CommandBuffer& command_buffer,
                         const vk::RenderPass& render_pass,
                         const IFramebuffer& framebuffer,
                         std::shared_ptr<IPipeline> pipeline) {
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

    void endRenderPass(const vk::CommandBuffer& command_buffer) {
        command_buffer.endRenderPass();
    }

    void endRecording(const vk::CommandBuffer& command_buffer) {
        command_buffer.end();
    }


    void recordBindResourceSetForCommand(const vk::CommandBuffer& command_buffer,
                                         std::shared_ptr<IResourceSetChain> resource_set,
                                         uint32_t binding,
                                         std::shared_ptr<IPipeline> pipeline,
                                         uint32_t index) {
        vk::DescriptorSet descriptor_set = resource_set->getResourceSetAt(index).getDescriptorSet();

        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                          pipeline->getPipelineInfo().layout,
                                          binding,
                                          1,
                                          &descriptor_set,
                                          0,
                                          nullptr);
    }

    void recordDrawForCommand(const vk::CommandBuffer& command_buffer,
                              const std::vector<std::shared_ptr<IBuffer>>& vertex_buffers,
                              const std::shared_ptr<IIndexBuffer> index_buffer,
                              uint32_t instance_count) {

        recordBindAndDrawVertexBuffers(command_buffer,
                                       vertex_buffers,
                                       index_buffer,
                                       instance_count);
    }

    void recordMakeTextureIntoFramebufferAndClear(const vk::CommandBuffer& command,
                                                  const IImage& image,
                                                  const vk::Device& device) {
        CommandLayoutTransitionData data;
        imageUtil::recordSetLayout(data,
                                   command,
                                   vk::ImageLayout::eUndefined,
                                   image.getIntendedLayout(),
                                   image);

    }

    void recordMakeFramebufferIntoTexture(const vk::CommandBuffer& command,
                                          const IImage& image,
                                          const vk::Device& device) {
        CommandLayoutTransitionData data;
        imageUtil::recordSetLayout(data,
                                   command,
                                   image.getIntendedLayout(),
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   image);
    }
};
