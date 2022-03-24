#include <vulkan/vulkan.hpp>

#include "../image/IImage.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal::framebufferUtil {

    vk::Framebuffer createBasicFramebuffer(const IImage& color_image,
                                           const IImage& depth_image,
                                           CompatibleRenderPassRegistry& render_pass_registry,
                                           const vk::Device& device);

    vk::Framebuffer createDepthOnlyFramebuffer(const IImage& depth_image,
                                               CompatibleRenderPassRegistry& render_pass_registry,
                                               const vk::Device& device);
};
