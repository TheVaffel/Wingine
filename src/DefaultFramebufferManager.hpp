#include <vulkan/vulkan.hpp>

#include <vector>

#include "./DeviceManager.hpp"
#include "./SwapchainManager.hpp"

// #include "./framebuffer.hpp"
#include "./framebuffer/IFramebuffer.hpp"
#include "./render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {
    class DefaultFramebufferManager {

        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const SwapchainManager> swapchain_manager;

        vk::Fence image_acquired_fence;
        vk::Semaphore image_acquire_semaphore;
        vk::Semaphore finished_drawing_semaphore;

        uint32_t current_swapchain_image;
        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;

        void initSyncStructs(const vk::Device& device);

    public:

        DefaultFramebufferManager(const vk::Extent2D& dimensions,
                                  const vk::SurfaceKHR& surface,
                                  std::shared_ptr<const DeviceManager> device_manager,
                                  const QueueManager& queue_manager,
                                  CompatibleRenderPassRegistry& render_pass_registry);
        ~DefaultFramebufferManager();

        void stageNextImage(const vk::Queue& queue,
                            const std::initializer_list<SemaphoreChain*>& semaphores);

        void present(const vk::Queue& present_queue,
                     const std::initializer_list<SemaphoreChain*>& semaphores);
        void waitForLastPresent();

        const std::vector<std::unique_ptr<IFramebuffer>>& getFramebuffers();
        const IFramebuffer& getCurrentFramebuffer() const;
        uint32_t getCurrentImageIndex() const;
    };
};
