#include <vulkan/vulkan.hpp>

#include <vector>

#include "../core/DeviceManager.hpp"
#include "../core/SwapchainManager.hpp"

#include "../framebuffer/FramebufferChainBase.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

#include "../util/SettableIndexCounter.hpp"

namespace wg::internal {
    class SwapchainFramebufferChain : public FramebufferChainBase {

        std::shared_ptr<const SwapchainManager> swapchain_manager;

        std::vector<vk::Fence> image_acquired_fences;
        std::vector<vk::Semaphore> image_acquired_semaphores;

        SettableIndexCounter swapchain_index_counter;
        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;

        void initSyncStructs(const vk::Device& device, const vk::Queue& queue);

        void stageNextImage();
        void present();

        bool hasSemaphoresToSignal() const;
        void runImageAcquisition(const vk::Semaphore& signal_semaphore);
        void signalImageAcquisitionSemaphores(uint32_t index);

    public:

        SwapchainFramebufferChain(const vk::Extent2D& dimensions,
                                  const vk::SurfaceKHR& surface,
                                  std::shared_ptr<const DeviceManager> device_manager,
                                  std::shared_ptr<const QueueManager> queue_manager,
                                  CompatibleRenderPassRegistry& render_pass_registry);
        ~SwapchainFramebufferChain();

        virtual void swapFramebuffer();
        virtual const IFramebuffer& getCurrentFramebuffer() const;

        virtual uint32_t getNumFramebuffers() const;

        virtual const IFramebuffer& getFramebuffer(uint32_t index) const;
    };
};
