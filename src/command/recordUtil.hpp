#pragma once

#include <vulkan/vulkan.hpp>

#include "../framebuffer/IFramebuffer.hpp"
#include "../pipeline/IPipeline.hpp"

#include "../buffer/IBuffer.hpp"
#include "../buffer/IIndexBuffer.hpp"

#include "./CommandControllerSettings.hpp"

#include "../resource/IResourceSetChain.hpp"

namespace wg::internal::recordUtil {

    void beginRecording(const vk::CommandBuffer& command);
    void beginRenderPass(const CommandControllerSettings& settings,
                         const vk::CommandBuffer& command,
                         const vk::RenderPass& render_pass,
                         const IFramebuffer& framebuffer,
                         std::shared_ptr<IPipeline> pipeline);

    void endRenderPass(const vk::CommandBuffer& command_buffer);
    void endRecording(const vk::CommandBuffer& command_buffer);


    void recordDrawForCommand(const vk::CommandBuffer& command_buffer,
                              std::shared_ptr<IPipeline> pipeline,
                              const std::vector<std::shared_ptr<IBuffer>>& vertex_buffers,
                              const std::shared_ptr<IIndexBuffer> index_buffer,
                              const std::vector<std::shared_ptr<IResourceSetChain>>& resource_sets,
                              uint32_t index,
                              uint32_t instance_count);

    void recordMakeFramebufferIntoTexture(const vk::CommandBuffer& command,
                                          const IImage& image,
                                          const vk::Device& device);

    void recordMakeTextureIntoFramebufferAndClear(const vk::CommandBuffer& command,
                                                  const IImage& image,
                                                  const vk::Device& device);
};
