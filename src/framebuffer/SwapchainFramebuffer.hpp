#include "./IFramebuffer.hpp"

#include "../SwapchainManager.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {
    class SwapchainFramebuffer : public IFramebuffer {

        std::unique_ptr<IImage> color_image;
        std::unique_ptr<IImage> depth_image;

        vk::Framebuffer framebuffer;

        std::shared_ptr<const SwapchainManager> swapchain_manager;
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
        virtual IImage& getColorImage();

        virtual bool hasDepthImage() const;
        virtual const IImage& getDepthImage() const;
        virtual IImage& getDepthImage();
    };
};
