#pragma once

#include <vulkan/vulkan.hpp>

#include "../framebuffer/IFramebuffer.hpp"
#include "../pipeline.hpp"

#include "./CommandControllerSettings.hpp"

#include "../resource/IResourceSetChain.hpp"

namespace wg::internal::recordUtil {
    /* void beginGraphicsCommand(const CommandControllerSettings& settings,
                              const vk::CommandBuffer& command,
                              const vk::RenderPass& render_pass,
                              const IFramebuffer& framebuffer,
                              const Pipeline* pipeline); */

    void beginRecording(const vk::CommandBuffer& command);
    void beginRenderPass(const CommandControllerSettings& settings,
                         const vk::CommandBuffer& command,
                         const vk::RenderPass& render_pass,
                         const IFramebuffer& framebuffer,
                         const Pipeline* pipeline);

    void endRenderPass(const vk::CommandBuffer& command_buffer);
    void endRecording(const vk::CommandBuffer& command_buffer);


    void recordDrawForCommand(const vk::CommandBuffer& command_buffer,
                              const Pipeline* pipeline,
                              const std::vector<const Buffer*>& vertex_buffers,
                              const IndexBuffer* index_buffer,
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
