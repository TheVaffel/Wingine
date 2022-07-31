#include "./FramebufferChainBase.hpp"

#include "../SwapchainManager.hpp"

#include "../image/SwapchainColorImage.hpp"
#include "../image/BasicImage.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {
    class SwapchainFramebuffer : public IFramebuffer {

        std::shared_ptr<const SwapchainManager> swapchain_manager;

        SwapchainColorImage color_image;
        BasicImage depth_image;

        vk::Framebuffer framebuffer;
        std::shared_ptr<const DeviceManager> device_manager;

        SwapchainFramebuffer(const vk::Image& image,
                             std::shared_ptr<const SwapchainManager> swapchain_manager,
                             std::shared_ptr<const DeviceManager> device_manager,
                             CompatibleRenderPassRegistry& render_pass_registry);
        ~SwapchainFramebuffer();

    public:

        static std::unique_ptr<IFramebuffer>
        createSwapchainFramebuffer(const vk::Image& image,
                                   std::shared_ptr<const SwapchainManager> swapchain_manager,
                                   std::shared_ptr<const DeviceManager> device_manager,
                                   CompatibleRenderPassRegistry& render_pass_registry);

        virtual vk::Extent2D getDimensions() const;

        virtual const vk::Framebuffer& getFramebuffer() const;

        virtual bool hasColorImage() const;
        virtual const IImage& getColorImage() const;

        virtual bool hasDepthImage() const;
        virtual const IImage& getDepthImage() const;
    };
};
