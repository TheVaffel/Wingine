#pragma once

#include "./IImage.hpp"
#include "../CommandManager.hpp"
#include "../sync/WaitSemaphoreSet.hpp"
#include "../sync/SignalSemaphoreSet.hpp"
#include "./CopyImageAuxillaryData.hpp"

namespace wg::internal {
    class ImageCopier {
        Command command;
        vk::Queue queue;
        std::shared_ptr<const CommandManager> command_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        WaitSemaphoreSet wait_semaphore_set;
        SignalSemaphoreSet signal_semaphore_set;

        CopyImageAuxillaryData auxillary_data;

    public:

        ImageCopier(const vk::Queue& queue,
                    std::shared_ptr<const CommandManager> command_manager,
                    std::shared_ptr<const DeviceManager> device_manager);

        void recordCopyImage(IImage& src,
                             IImage& dst);

        void setWaitSemaphoreSet(const WaitSemaphoreSet& semaphores);
        void setSignalSemaphoreSet(const SignalSemaphoreSet& semaphores);

        void runCopy();
        void awaitCopy();
        void runAndAwaitCopy();

        vk::Fence getImageCopyCompleteFence();

        ~ImageCopier();
    };
};
