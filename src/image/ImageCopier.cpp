#include "./ImageCopier.hpp"

#include "./copyImage.hpp"
#include "../sync/fenceUtil.hpp"
#include "../util/log.hpp"

#include <iostream>

namespace wg::internal {
    ImageCopier::ImageCopier(const vk::Queue& queue,
                             std::shared_ptr<const CommandManager> command_manager,
                             std::shared_ptr<const DeviceManager> device_manager)
        : SynchronizedQueueOperationBase(1, device_manager),
          queue(queue),
          command_manager(command_manager),
          device_manager(device_manager) {

        this->command = command_manager->createGraphicsCommands(1)[0];
    }

    void ImageCopier::recordCopyImage(IImage& src,
                                      IImage& dst) {
        copyImage::recordCopyImage(src,
                                   dst,
                                   this->auxillary_data,
                                   this->command,
                                   this->device_manager->getDevice(),
                                   this->queue);
    }

    void ImageCopier::runCopy() {

        std::vector<vk::PipelineStageFlags> wait_stages(this->getSemaphores().getWaitSemaphores().getNumSemaphores(),
                                                        vk::PipelineStageFlagBits::eTopOfPipe);

        vk::SubmitInfo submit_info;
        submit_info.setCommandBufferCount(1)
            .setPCommandBuffers(&this->command.buffer)
            .setPWaitDstStageMask(wait_stages.data())
            .setWaitSemaphoreCount(this->getSemaphores().getWaitSemaphores().getNumSemaphores())
            .setPWaitSemaphores(this->getSemaphores().getWaitSemaphores().getCurrentRawSemaphores().data())
            .setSignalSemaphores(this->getSemaphores().getSignalSemaphores().getCurrentRawSemaphores());

        this->getSemaphores().getSignalSemaphores().swapSemaphores();
        this->getSemaphores().getWaitSemaphores().swapSemaphores();

        _wassert_result(this->queue.submit(1, &submit_info, this->command.fence),
                        "command submission from ImageCopier");
    }


    void ImageCopier::awaitCopy() {
        fenceUtil::awaitAndResetFence(this->command.fence,
                                      this->device_manager->getDevice());
    }

    void ImageCopier::runAndAwaitCopy() {
        this->runCopy();
        this->awaitCopy();
    }


    vk::Fence ImageCopier::getImageCopyCompleteFence() {
        return this->command.fence;
    }

    ImageCopier::~ImageCopier() {
        this->command_manager->destroyGraphicsCommands({this->command});
    }
};
