#include "./IImage.hpp"
#include "../CommandManager.hpp"
#include "../sync/SemaphoreSet.hpp"
#include "./CopyImageAuxillaryData.hpp"

namespace wg::internal {
    class ImageCopier {
        Command command;
        vk::Queue queue;
        std::shared_ptr<const CommandManager> command_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        SemaphoreSet wait_semaphore_set;
        SemaphoreSet signal_semaphore_set;

        CopyImageAuxillaryData auxillary_data;

    public:

        ImageCopier(const vk::Queue& queue,
                    std::shared_ptr<const CommandManager> command_manager,
                    std::shared_ptr<const DeviceManager> device_manager);

        void recordCopyImage(IImage& src,
                             IImage& dst);

        void setWaitSemaphoreSet(const SemaphoreSet& semaphores);
        void setSignalSemaphoreSet(const SemaphoreSet& semaphores);

        void runCopy();
        void awaitCopy();
        void runAndAwaitCopy();

        ~ImageCopier();
    };
};
