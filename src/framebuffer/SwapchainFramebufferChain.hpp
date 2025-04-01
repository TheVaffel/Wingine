#include <vulkan/vulkan.hpp>

#include <vector>
#include <queue>

#include "../core/DeviceManager.hpp"
#include "../core/SwapchainManager.hpp"

#include "../framebuffer/FramebufferChainBase.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include "../util/IndexCounter.hpp"

namespace wg::internal {
    class SwapchainFramebufferChain : public FramebufferChainBase {

        std::shared_ptr<const SwapchainManager> swapchain_manager;

        vk::Fence image_acquired_fence;

        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;

        std::queue<uint32_t> next_image_index_to_present_queue;

        void initSyncStructs(const vk::Device& device);

        void stageNextImage();
        void present();

        void runImageAcquisition();

    public:

        SwapchainFramebufferChain(const vk::Extent2D& dimensions,
                                  const vk::SurfaceKHR& surface,
                                  std::shared_ptr<const DeviceManager> device_manager,
                                  std::shared_ptr<const QueueManager> queue_manager,
                                  CompatibleRenderPassRegistry& render_pass_registry);
        ~SwapchainFramebufferChain();

        virtual const IFramebuffer& getCurrentFramebuffer() const final;

        virtual void swapToNextElement() final;
        virtual uint32_t getCurrentElementIndex() const final;
        virtual uint32_t getElementChainLength() const final;


        virtual const IFramebuffer& getFramebuffer(uint32_t index) const final;
    };
};
