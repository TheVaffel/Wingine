#include <vulkan/vulkan.hpp>

#include "../image/IImage.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal::framebufferUtil {

    vk::Framebuffer createBasicFramebuffer(const IImage& colorImage,
                                           const IImage& depthImage,
                                           CompatibleRenderPassRegistry& render_pass_registry,
                                           const vk::Device& device);
};
