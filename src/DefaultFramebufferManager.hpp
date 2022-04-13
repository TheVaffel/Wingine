#include <vulkan/vulkan.hpp>

#include <vector>

#include "./DeviceManager.hpp"
#include "./SwapchainManager.hpp"

// #include "./framebuffer.hpp"
#include "./framebuffer/IFramebuffer.hpp"
#include "./framebuffer/IFramebufferChain.hpp"
#include "./render_pass/CompatibleRenderPassRegistry.hpp"

#include "./sync/SemaphoreSet.hpp"

namespace wg::internal {
    class DefaultFramebufferManager : public IFramebufferChain {

        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const QueueManager> queue_manager;
        std::shared_ptr<const SwapchainManager> swapchain_manager;

        std::vector<vk::Fence> image_acquired_fences;
        std::vector<vk::Semaphore> image_acquired_semaphores;

        SemaphoreSet wait_before_present_semaphore_set;
        SemaphoreSet signal_on_image_acquired_semaphore_set;

        uint32_t current_swapchain_image;
        std::vector<std::unique_ptr<IFramebuffer>> framebuffers;

        void initSyncStructs(const vk::Device& device, const vk::Queue& queue);

        void stageNextImage();
        void present();

        bool hasSemaphoresToSignal() const;
        void runImageAcquisition(const vk::Semaphore& signal_semaphore);
        void signalImageAcquisitionSemaphores(uint32_t index);

    public:

        DefaultFramebufferManager(const vk::Extent2D& dimensions,
                                  const vk::SurfaceKHR& surface,
                                  std::shared_ptr<const DeviceManager> device_manager,
                                  std::shared_ptr<const QueueManager> queue_manager,
                                  CompatibleRenderPassRegistry& render_pass_registry);
        ~DefaultFramebufferManager();

        void setPresentWaitSemaphores(const SemaphoreSet& semaphores);
        std::shared_ptr<ManagedSemaphoreChain> addSignalImageAcquiredSemaphore();
        void setSignalImageAcquiredSemaphores(const SemaphoreSet& semaphores);

        const std::vector<std::unique_ptr<IFramebuffer>>& getFramebuffers();


        virtual void swapFramebuffer();
        virtual const IFramebuffer& getCurrentFramebuffer() const;
        virtual IFramebuffer& getCurrentFramebuffer();

        virtual uint32_t getNumFramebuffers() const;

        virtual const IFramebuffer& getFramebuffer(uint32_t index) const;
        virtual IFramebuffer& getFramebuffer(uint32_t index);
    };
};
