#pragma once

#include "./IImage.hpp"
#include "../CommandManager.hpp"
#include "../sync/SemaphoreSet.hpp"
#include "./CopyImageAuxillaryData.hpp"

#include "../util/IndexCounter.hpp"

#include "../buffer/IBuffer.hpp"

namespace wg::internal {
    class ImageToBufferCopier {
        std::vector<Command> commands;
        vk::Queue queue;
        std::shared_ptr<const CommandManager> command_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        IndexCounter current_image_counter;

        SemaphoreSet wait_semaphore_set;
        SemaphoreSet signal_semaphore_set;

        CopyImageToBufferAuxillaryData auxillary_data;

    public:

        ImageToBufferCopier(uint32_t num_images,
                            const vk::Queue& queue,
                            std::shared_ptr<const CommandManager> command_manager,
                            std::shared_ptr<const DeviceManager> device_manager);

        void recordCopyImage(const std::vector<IImage*>& srcs,
                             const std::vector<IBuffer*>& dsts);

        void setWaitSemaphoreSet(const SemaphoreSet& semaphores);
        void setSignalSemaphoreSet(const SemaphoreSet& semaphores);
        std::shared_ptr<ManagedSemaphoreChain> addSignalSemaphore();

        void runCopy();
        void awaitPreviousCopy();
        void runAndAwaitCopy();

        vk::Fence getLastImageCopyCompleteFence();

        ~ImageToBufferCopier();
    };
};
