#include "./DefaultFramebufferManager.hpp"

#include "./framebuffer/SwapchainFramebuffer.hpp"
#include "./sync/semaphoreUtil.hpp"
#include "./sync/fenceUtil.hpp"
#include "./log.hpp"

#include <iostream>

namespace wg::internal {

    DefaultFramebufferManager::DefaultFramebufferManager(const vk::Extent2D& dimensions,
                                                         const vk::SurfaceKHR& surface,
                                                         std::shared_ptr<const DeviceManager> device_manager,
                                                         std::shared_ptr<const QueueManager> queue_manager,
                                                         CompatibleRenderPassRegistry& render_pass_registry)
        : device_manager(device_manager),
          queue_manager(queue_manager),
          swapchain_manager(std::make_shared<SwapchainManager>(dimensions,
                                                               surface,
                                                               device_manager,
                                                               *queue_manager)),
          wait_before_present_semaphore_set(swapchain_manager->getNumImages(), device_manager),
          signal_on_image_acquired_semaphore_set(swapchain_manager->getNumImages(), device_manager),
          current_swapchain_image(0) {

        this->swapchain_manager = std::make_shared<SwapchainManager>(dimensions,
                                                                     surface,
                                                                     device_manager,
                                                                     *queue_manager);

        this->initSyncStructs(device_manager->getDevice(),
                              queue_manager->getPresentQueue());

        for (uint32_t i = 0; i < swapchain_manager->getNumImages(); i++) {
            this->framebuffers.push_back(SwapchainFramebuffer::createSwapchainFramebuffer(
                                             swapchain_manager->getImages()[i],
                                             swapchain_manager,
                                             device_manager,
                                             render_pass_registry));
        }

        this->stageNextImage();
    }



    void DefaultFramebufferManager::initSyncStructs(const vk::Device& device,
                                                    const vk::Queue& queue) {
        vk::FenceCreateInfo fence_create_info;
        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);
        vk::SemaphoreCreateInfo semaphore_create_info;

        for (uint32_t i = 0; i < this->swapchain_manager->getNumImages(); i++) {
            this->image_acquired_fences.push_back(device.createFence(fence_create_info));

            vk::Semaphore image_acquired_semaphore = device.createSemaphore(semaphore_create_info);
            this->image_acquired_semaphores.push_back(image_acquired_semaphore);
        }
    }

    void DefaultFramebufferManager::setPresentWaitSemaphores(const WaitSemaphoreSet& semaphores) {
        this->wait_before_present_semaphore_set = semaphores;
    }

    std::shared_ptr<ManagedSemaphoreChain> DefaultFramebufferManager::addSignalImageAcquiredSemaphore() {
        return this->signal_on_image_acquired_semaphore_set
            .addSignalledSemaphoreChain(this->queue_manager->getPresentQueue());
    }

    void DefaultFramebufferManager::setSignalImageAcquiredSemaphores(const SignalSemaphoreSet& semaphores) {
        this->signal_on_image_acquired_semaphore_set = semaphores;
    }

    void DefaultFramebufferManager::present() {
        #ifdef DEBUG
        if(!this->wait_before_present_semaphore_set.getNumSemaphores()) {
            std::cerr << "[Wingine::present] Warning: No semaphore submitted to present(), "
                      << "presentation may not happen correctly"
                      << std::endl;
        }
#endif // DEBUG

        vk::PresentInfoKHR presentInfo;

        uint32_t swapchain_image_index = this->current_swapchain_image;

        presentInfo.setSwapchainCount(1)
            .setPSwapchains(&this->swapchain_manager->getSwapchain())
            .setPImageIndices(&swapchain_image_index)
            .setWaitSemaphoreCount(this->wait_before_present_semaphore_set.getNumSemaphores())
            .setPWaitSemaphores(this->wait_before_present_semaphore_set
                                .getCurrentRawSemaphores().data())
            .setPResults(nullptr);

        this->wait_before_present_semaphore_set.swapSemaphores();

        _wassert_result(this->queue_manager->getPresentQueue().presentKHR(presentInfo),
                        "submit present command");

        this->stageNextImage();
    }

    bool DefaultFramebufferManager::hasSemaphoresToSignal() const {
        return this->signal_on_image_acquired_semaphore_set
            .getCurrentRawSemaphores().size() != 0;
    }

    void DefaultFramebufferManager::runImageAcquisition(const vk::Semaphore& signal_semaphore ) {

        _wassert_result(this->device_manager->getDevice()
                        .acquireNextImageKHR(this->swapchain_manager->getSwapchain(),
                                             UINT64_MAX,
                                             signal_semaphore,
                                             this->image_acquired_fences[this->current_swapchain_image],
                                             &(this->current_swapchain_image)),
                        "acquiring next image");

    }

    void DefaultFramebufferManager::signalImageAcquisitionSemaphores(uint32_t index) {
            semaphoreUtil::signalManySemaphoresFromSingleSemaphore(
                this->signal_on_image_acquired_semaphore_set.getCurrentRawSemaphores(),
                this->image_acquired_semaphores[index],
                this->queue_manager->getPresentQueue());
            this->signal_on_image_acquired_semaphore_set.swapSemaphores();
    }

    void DefaultFramebufferManager::stageNextImage() {

        fenceUtil::awaitAndResetFence(this->image_acquired_fences[this->current_swapchain_image],
                                      this->device_manager->getDevice());

        uint32_t currently_acquiring_image_index = this->current_swapchain_image;

        bool should_signal_semaphore = this->hasSemaphoresToSignal();

        vk::Semaphore signal_semaphore = should_signal_semaphore ?
            this->image_acquired_semaphores[this->current_swapchain_image] : nullptr;

        this->runImageAcquisition(signal_semaphore);

        if (should_signal_semaphore) {
            this->signalImageAcquisitionSemaphores(currently_acquiring_image_index);
        }
    }

    void DefaultFramebufferManager::swapFramebuffer() {
        this->present();
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

    DefaultFramebufferManager::~DefaultFramebufferManager() {
        const vk::Device& device = this->device_manager->getDevice();

        for (uint32_t i = 0; i < this->swapchain_manager->getNumImages(); i++) {
            fenceUtil::awaitFence(this->image_acquired_fences[i], device);
            device.destroy(this->image_acquired_fences[i]);
            device.destroy(this->image_acquired_semaphores[i]);
        }
    }
};
