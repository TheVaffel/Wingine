#include "./SwapchainFramebufferChain.hpp"

#include "../framebuffer/SwapchainFramebuffer.hpp"
#include "../sync/fenceUtil.hpp"
#include "../util/log.hpp"

#include <iostream>

namespace wg::internal {

    SwapchainFramebufferChain::SwapchainFramebufferChain(const vk::Extent2D& dimensions,
                                                         const vk::SurfaceKHR& surface,
                                                         std::shared_ptr<const DeviceManager> device_manager,
                                                         std::shared_ptr<const QueueManager> queue_manager,
                                                         CompatibleRenderPassRegistry& render_pass_registry)
    : FramebufferChainBase(SwapchainManager::getNumFramebuffers(surface, device_manager),
                           queue_manager,
                           device_manager),
      swapchain_manager(std::make_shared<SwapchainManager>(dimensions,
                                                           surface,
                                                           device_manager,
                                                           *queue_manager)) {

        this->initSyncStructs(device_manager->getDevice());

        for (uint32_t i = 0; i < swapchain_manager->getNumImages(); i++) {
            this->framebuffers.push_back(SwapchainFramebuffer::createSwapchainFramebuffer(
                                             swapchain_manager->getImages()[i],
                                             swapchain_manager,
                                             device_manager,
                                             render_pass_registry));
        }

        this->stageNextImage();
    }



    void SwapchainFramebufferChain::initSyncStructs(const vk::Device& device) {
        vk::FenceCreateInfo fence_create_info;
        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->image_acquired_fence = device.createFence(fence_create_info);

        fenceUtil::resetFence(this->image_acquired_fence, this->device_manager->getDevice());
    }

    void SwapchainFramebufferChain::present() {
        #ifdef DEBUG
        if(!this->getWaitSemaphores().getNumSemaphores()) {
            std::cerr << "[Wingine::present] Warning: No semaphore submitted to present(), "
                      << "presentation may not happen correctly"
                      << std::endl;
        }
#endif // DEBUG

        vk::PresentInfoKHR presentInfo;

        uint32_t swapchain_image_index = this->next_image_index_to_present_queue.front();
        this->next_image_index_to_present_queue.pop();

        presentInfo.setSwapchainCount(1)
            .setPSwapchains(&this->swapchain_manager->getSwapchain())
            .setPImageIndices(&swapchain_image_index)
            .setWaitSemaphoreCount(this->getWaitSemaphores().getNumSemaphores())
            .setPWaitSemaphores(this->getWaitSemaphores()
                                .getCurrentRawSemaphores().data())
            .setPResults(nullptr);

        this->getWaitSemaphores().swapSemaphores();

        _wassert_result(this->queue_manager->getPresentQueue().presentKHR(presentInfo),
                        "submit present command");

        this->stageNextImage();
    }

    void SwapchainFramebufferChain::runImageAcquisition( ) {
        uint32_t swapchain_index;

        _wassert_result(this->device_manager->getDevice()
                        .acquireNextImageKHR(this->swapchain_manager->getSwapchain(),
                                             UINT64_MAX,
                                             nullptr,
                                             this->image_acquired_fence,
                                             &(swapchain_index)),
                        "acquiring next image");

        fenceUtil::awaitAndResetFence(this->image_acquired_fence,
                                      this->device_manager->getDevice());

        this->next_image_index_to_present_queue.push(swapchain_index);
    }

    void SwapchainFramebufferChain::stageNextImage() {
        this->runImageAcquisition();
    }

    void SwapchainFramebufferChain::swapToNextElement() {
        this->present();
    }

    const IFramebuffer& SwapchainFramebufferChain::getCurrentFramebuffer() const {
        return *this->framebuffers[this->next_image_index_to_present_queue.front()];
    }

    uint32_t SwapchainFramebufferChain::getCurrentElementIndex() const {
        return this->next_image_index_to_present_queue.front();
    }

    uint32_t SwapchainFramebufferChain::getElementChainLength() const {
        return this->framebuffers.size();
    }

    const IFramebuffer& SwapchainFramebufferChain::getFramebuffer(uint32_t index) const {
        return *this->framebuffers[index];
    }

    SwapchainFramebufferChain::~SwapchainFramebufferChain() {
        const vk::Device& device = this->device_manager->getDevice();

        device.destroy(this->image_acquired_fence);
    }
};
