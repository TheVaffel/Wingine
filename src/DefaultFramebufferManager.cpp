#include "./DefaultFramebufferManager.hpp"

#include "./framebuffer/SwapchainFramebuffer.hpp"
#include "./log.hpp"

#include <iostream>

namespace wg::internal {

    DefaultFramebufferManager::DefaultFramebufferManager(const vk::Extent2D& dimensions,
                                                         const vk::SurfaceKHR& surface,
                                                         std::shared_ptr<const DeviceManager> device_manager,
                                                         const QueueManager& queue_manager,
                                                         CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager) {

        this->swapchain_manager = std::make_shared<SwapchainManager>(dimensions,
                                                                     surface,
                                                                     device_manager,
                                                                     queue_manager);


        this->initSyncStructs(device_manager->getDevice());

        for (uint32_t i = 0; i < swapchain_manager->getNumImages(); i++) {
            this->framebuffers.push_back(SwapchainFramebuffer::createSwapchainFramebuffer(
                                             swapchain_manager->getImages()[i],
                                             swapchain_manager,
                                             device_manager,
                                             render_pass_registry));
        }
    }



    void DefaultFramebufferManager::initSyncStructs(const vk::Device& device) {
        vk::FenceCreateInfo fence_create_info;
        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->image_acquired_fence = device.createFence(fence_create_info);

        vk::SemaphoreCreateInfo semaphore_create_info;
        this->image_acquire_semaphore = device.createSemaphore(semaphore_create_info);
        this->finished_drawing_semaphore = device.createSemaphore(semaphore_create_info);
    }

    void DefaultFramebufferManager::present(const vk::Queue& present_queue,
                                            const std::initializer_list<SemaphoreChain*>& semaphores) {
        #ifdef DEBUG
        if(!semaphores.size()) {
            std::cout << "[Wingine::present] Warning: No semaphore submitted to present(), presentation may not happen correctly"
                      << std::endl;
        }
#endif // DEBUG

        vk::PresentInfoKHR presentInfo;

        SemaphoreChain::chainsToSemaphore(present_queue,
                                          std::begin(semaphores),
                                          semaphores.size(),
                                          this->finished_drawing_semaphore);

        // Present, but wait for finished_drawing_semaphore, which waits on the rest of the semaphores

        uint32_t swapchain_image_index = this->current_swapchain_image;

        presentInfo.setSwapchainCount(1)
            .setPSwapchains(&this->swapchain_manager->getSwapchain())
            .setPImageIndices(&swapchain_image_index)
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&this->finished_drawing_semaphore)
            .setPResults(nullptr);

        std::cout << "[DefaultFramebufferManager.cpp] Presenting KHR" << std::endl;
        _wassert_result(present_queue.presentKHR(presentInfo),
                        "submit present command");
        std::cout << "[DefaultFramebufferManager.cpp] Done presenting" << std::endl;

        this->stageNextImage(present_queue, semaphores);

        SemaphoreChain::resetModifiers(std::begin(semaphores), semaphores.size());
    }


    const std::vector<std::unique_ptr<IFramebuffer>>& DefaultFramebufferManager::getFramebuffers() {
        return this->framebuffers;
    }

    uint32_t DefaultFramebufferManager::getCurrentImageIndex() const {
        return this->current_swapchain_image;
    }

    void DefaultFramebufferManager::waitForLastPresent() {
        std::cout << "waiting for fence in waitForLastPresent" << std::endl;
        _wassert_result(this->device_manager->getDevice()
                        .waitForFences(1, &this->image_acquired_fence, true, UINT64_MAX),
                        "wait for last present");

        std::cout << "Finished waiting" << std::endl;

    }


    void DefaultFramebufferManager::stageNextImage(const vk::Queue& queue,
                                                   const std::initializer_list<SemaphoreChain*>& semaphores) {
        int num_semaphores = semaphores.size();

        this->waitForLastPresent();
        _wassert_result(this->device_manager->getDevice().resetFences(1, &this->image_acquired_fence),
                        "reset fence in stage_next_image");

        _wassert_result(this->device_manager->getDevice()
                        .acquireNextImageKHR(this->swapchain_manager->getSwapchain(),
                                             UINT64_MAX,
                                             num_semaphores
                                             ? this->image_acquire_semaphore
                                             : vk::Semaphore(nullptr),
                                             image_acquired_fence,
                                             &(this->current_swapchain_image)),
                        "acquiring next image");

        if(num_semaphores) {
            SemaphoreChain::semaphoreToChains(queue, this->image_acquire_semaphore, std::begin(semaphores), num_semaphores);
        }
    }



    DefaultFramebufferManager::~DefaultFramebufferManager() {
        const vk::Device& device = this->device_manager->getDevice();

        device.destroyFence(this->image_acquired_fence);
        device.destroy(this->image_acquire_semaphore);
        device.destroy(this->finished_drawing_semaphore);
    }



    void DefaultFramebufferManager::swapFramebuffer() {
        // Do nothing
    }

    const IFramebuffer& DefaultFramebufferManager::getCurrentFramebuffer() const {
        return *this->framebuffers[this->current_swapchain_image];
    }

    IFramebuffer& DefaultFramebufferManager::getCurrentFramebuffer() {
        return *this->framebuffers[this->current_swapchain_image];
    }

    uint32_t DefaultFramebufferManager::getNumFramebuffers() const {
        return this->framebuffers.size();
    }

    const IFramebuffer& DefaultFramebufferManager::getFramebuffer(uint32_t index) const {
        return *this->framebuffers[index];
    }

    IFramebuffer& DefaultFramebufferManager::getFramebuffer(uint32_t index) {
        return *this->framebuffers[index];
    }
};
