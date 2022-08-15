#include "./ImageChainCopier.hpp"

#include <flawed_assert.hpp>

#include "./copyImage.hpp"
#include "../util/log.hpp"
#include "../sync/fenceUtil.hpp"

#include "../framebuffer/IFramebufferChain.hpp"

namespace wg::internal {
    ImageChainCopier::ImageChainCopier(uint32_t count,
                     const vk::Queue& queue,
                     std::shared_ptr<const CommandManager> command_manager,
                     std::shared_ptr<const DeviceManager> device_manager)
        : image_index(count),
          semaphores(count, device_manager),
          device_manager(device_manager),
          command_manager(command_manager),
          queue(queue) {
        this->commands = command_manager->createGraphicsCommands(count);
        this->auxillary_data.resize(count);
    }

    void ImageChainCopier::recordCopies(const std::vector<IImage*>& srcs,
                                        const std::vector<IImage*>& dsts) {
        fl_assert_eq(srcs.size(), dsts.size());
        fl_assert_eq(srcs.size(), this->commands.size());

        for (uint32_t i = 0; i < srcs.size(); i++) {
            copyImage::recordCopyImage(*srcs[i],
                                       *dsts[i],
                                       this->auxillary_data[i],
                                       this->commands[i],
                                       this->device_manager->getDevice(),
                                       this->queue);
        }
    }

    void ImageChainCopier::recordCopies(std::shared_ptr<IFramebufferChain> framebuffer_chain,
                                        std::shared_ptr<ITextureChain> texture_chain) {
        fl_assert_eq(framebuffer_chain->getElementChainLength(),
                     texture_chain->getElementChainLength());

        std::vector<IImage*> srcs, dsts;
        for (uint32_t i = 0; i < texture_chain->getElementChainLength(); i++) {
            IImage& image = framebuffer_chain->getFramebuffer(i).hasColorImage() ?
                framebuffer_chain->getFramebuffer(i).getColorImage() :
                framebuffer_chain->getFramebuffer(i).getDepthImage();

            srcs.push_back(&image);
            dsts.push_back(&texture_chain->getTextureAt(i));
        }

        this->recordCopies(srcs, dsts);
    }

    void ImageChainCopier::runCopyAndSwap() {

        fenceUtil::awaitAndResetFence(this->commands[this->image_index.getCurrentIndex()].fence,
                                      this->device_manager->getDevice());

        std::vector<vk::PipelineStageFlags> wait_stages(this->getSemaphores().getWaitSemaphores().getNumSemaphores(),
                                                        vk::PipelineStageFlagBits::eTopOfPipe);

        vk::SubmitInfo submit_info;
        submit_info.setCommandBufferCount(1)
            .setPCommandBuffers(&this->commands[image_index.getCurrentIndex()].buffer)
            .setPWaitDstStageMask(wait_stages.data())
            .setWaitSemaphoreCount(this->getSemaphores().getWaitSemaphores().getNumSemaphores())
            .setPWaitSemaphores(this->getSemaphores().getWaitSemaphores().getCurrentRawSemaphores().data())
            .setSignalSemaphores(this->getSemaphores().getSignalSemaphores().getCurrentRawSemaphores());

        _wassert_result(this->queue.submit(1,
                                           &submit_info,
                                           this->commands[this->image_index.getCurrentIndex()].fence),
                        "command submission from ImageCopier");

        this->image_index.incrementIndex();

        this->getSemaphores().getSignalSemaphores().swapSemaphores();
        this->getSemaphores().getWaitSemaphores().swapSemaphores();
    }

    SignalAndWaitSemaphores& ImageChainCopier::getSemaphores() {

        return this->semaphores;
    }

    ImageChainCopier::~ImageChainCopier() {
        this->command_manager->destroyGraphicsCommands(this->commands);
    }
}
