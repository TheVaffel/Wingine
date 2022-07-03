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
            std::vector<vk::ClearValue> clear_values(2);

            if (settings.command_render_pass_settings.getNumColorAttachments() > 0) {
                if (settings.command_render_pass_settings.getShouldClearColor()) {
                    clear_values[0].color.setFloat32({ 0.2f, 0.2f, 0.2f, 1.0f });
                }
                if (settings.command_render_pass_settings.getShouldClearDepth()) {
                    clear_values[1].depthStencil.depth = 1.0f;
                    clear_values[1].depthStencil.stencil = 0.0f;
                }
            } else {
                if (settings.command_render_pass_settings.getShouldClearDepth()) {
                    clear_values[0].depthStencil.depth = 1.0f;
                    clear_values[0].depthStencil.stencil = 0.0f;
                }
            }

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
                                      const std::vector<std::shared_ptr<IResourceSetChain>>& resource_sets,
                                      uint32_t index,
                                      const Pipeline* pipeline) {

            std::vector<vk::DescriptorSet> descriptor_sets(resource_sets.size());
            for (uint32_t i = 0; i < resource_sets.size(); i++) {
                descriptor_sets[i] = resource_sets[i]->getResourceSetAt(index).getDescriptorSet();
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
    };

    /* void beginGraphicsCommand(const CommandControllerSettings& settings,
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
                                        } */

    void beginRecording(const vk::CommandBuffer& command_buffer) {
        vk::CommandBufferBeginInfo begin;
        command_buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        command_buffer.begin(begin);

    }

    void beginRenderPass(const CommandControllerSettings& settings,
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

    void endRenderPass(const vk::CommandBuffer& command_buffer) {
        command_buffer.endRenderPass();
    }

    void endRecording(const vk::CommandBuffer& command_buffer) {
        command_buffer.end();
    }

    void recordDrawForCommand(const vk::CommandBuffer& command_buffer,
                              const Pipeline* pipeline,
                              const std::vector<const Buffer*>& vertex_buffers,
                              const IndexBuffer* index_buffer,
                              const std::vector<std::shared_ptr<IResourceSetChain>>& resource_sets,
                              uint32_t index,
                              uint32_t instance_count) {

        recordBindDescriptorSets(command_buffer, resource_sets, index, pipeline);

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
