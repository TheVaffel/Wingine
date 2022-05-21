#pragma once

#include "./IImage.hpp"
#include "../CommandManager.hpp"
#include "../sync/SynchronizedQueueOperationBase.hpp"
#include "./CopyImageAuxillaryData.hpp"

namespace wg::internal {
    class ImageCopier : public SynchronizedQueueOperationBase {
        Command command;
        vk::Queue queue;
        std::shared_ptr<const CommandManager> command_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        CopyImageAuxillaryData auxillary_data;

    public:

        ImageCopier(const vk::Queue& queue,
                    std::shared_ptr<const CommandManager> command_manager,
                    std::shared_ptr<const DeviceManager> device_manager);

        void recordCopyImage(IImage& src,
                             IImage& dst);

        void runCopy();
        void awaitCopy();
        void runAndAwaitCopy();

        vk::Fence getImageCopyCompleteFence();

        ~ImageCopier();
    };
};
