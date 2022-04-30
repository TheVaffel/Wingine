#include "./ImageToBufferCopier.hpp"

#include "./copyImage.hpp"
#include "../sync/fenceUtil.hpp"
#include "../sync/semaphoreUtil.hpp"
#include "../util/log.hpp"

#include <iostream>

namespace wg::internal {
    ImageToBufferCopier::ImageToBufferCopier(uint32_t num_images,
                                             const vk::Queue& queue,
                                             std::shared_ptr<const CommandManager> command_manager,
                                             std::shared_ptr<const DeviceManager> device_manager)
        : queue(queue),
          command_manager(command_manager),
          device_manager(device_manager),
          current_image_counter(num_images),
          wait_semaphore_set(num_images, device_manager),
          signal_semaphore_set(num_images, device_manager) {

        this->commands = command_manager->createGraphicsCommands(num_images);
    }

    void ImageToBufferCopier::recordCopyImage(const std::vector<IImage*>& srcs,
                                              const std::vector<IBuffer*>& dsts) {
        for (uint32_t i = 0; i < srcs.size(); i++) {
            copyImage::recordCopyImageToBuffer(*srcs[i],
                                               *dsts[i],
                                               this->auxillary_data,
                                               this->commands[i],
                                               this->device_manager->getDevice(),
                                               this->queue);
        }
    }

    void ImageToBufferCopier::runCopy() {

        std::vector<vk::PipelineStageFlags> wait_stages(this->wait_semaphore_set.getNumSemaphores(),
                                                        vk::PipelineStageFlagBits::eTopOfPipe);

        fenceUtil::awaitAndResetFence(this->commands[this->current_image_counter.getCurrentIndex()].fence,
                                      this->device_manager->getDevice());

        vk::SubmitInfo submit_info;
        submit_info.setCommandBufferCount(1)
            .setPCommandBuffers(&this->commands[this->current_image_counter.getCurrentIndex()].buffer)
            .setPWaitDstStageMask(wait_stages.data())
            .setWaitSemaphoreCount(this->wait_semaphore_set.getNumSemaphores())
            .setPWaitSemaphores(this->wait_semaphore_set.getCurrentRawSemaphores().data())
            .setSignalSemaphores(this->signal_semaphore_set.getCurrentRawSemaphores());

        _wassert_result(this->queue.submit(1,
                                           &submit_info,
                                           this->commands[this->current_image_counter.getCurrentIndex()].fence),
                        "command submission from ImageToBufferCopier");

        this->wait_semaphore_set.swapSemaphores();
        this->signal_semaphore_set.swapSemaphores();
        this->current_image_counter.incrementIndex();
    }


    void ImageToBufferCopier::setWaitSemaphoreSet(const WaitSemaphoreSet& semaphores) {
        this->wait_semaphore_set = semaphores;
    }

    void ImageToBufferCopier::setSignalSemaphoreSet(const SignalSemaphoreSet& semaphores) {
        this->signal_semaphore_set = semaphores;
    }

    std::shared_ptr<ManagedSemaphoreChain> ImageToBufferCopier::addSignalSemaphore() {
        return this->signal_semaphore_set.addSignalledSemaphoreChain(this->queue);
    }

    void ImageToBufferCopier::awaitPreviousCopy() {
        fenceUtil::awaitAndResetFence(this->commands[this->current_image_counter.getPreviousIndex()].fence,
                                      this->device_manager->getDevice());
    }

    void ImageToBufferCopier::runAndAwaitCopy() {
        this->runCopy();
        this->awaitPreviousCopy();
    }


    vk::Fence ImageToBufferCopier::getLastImageCopyCompleteFence() {
        return this->commands[this->current_image_counter.getPreviousIndex()].fence;
    }

    ImageToBufferCopier::~ImageToBufferCopier() {
        this->command_manager->destroyGraphicsCommands(this->commands);
    }
};
