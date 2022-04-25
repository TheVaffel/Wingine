#include "./IFramebufferChain.hpp"

#include "../image/HostVisibleImageView.hpp"
#include "../image/ImageToBufferCopier.hpp"

#include "./BasicFramebufferChain.hpp"
#include "./BasicFramebuffer.hpp"

#include "../util/IndexCounter.hpp"

namespace wg::internal {

    class HostCopyingFramebufferChain : public IFramebufferChain {

        std::unique_ptr<ImageToBufferCopier> image_copier;

        IndexCounter inner_framebuffer_index_counter;

        std::shared_ptr<ManagedSemaphoreChain> semaphore_chain;
        SemaphoreSet image_copy_signal_semaphore_set;
        SemaphoreSet image_copy_wait_semaphore_set;

        std::shared_ptr<HostVisibleImageView> dst_image;
        std::shared_ptr<const QueueManager> queue_manager;
        std::shared_ptr<const DeviceManager> device_manager;

        BasicFramebufferChain<BasicFramebuffer> inner_framebuffer_chain;

    public:
        HostCopyingFramebufferChain(uint32_t num_framebuffers,
                                    std::shared_ptr<HostVisibleImageView> dst_image_view,
                                    std::shared_ptr<const QueueManager> queue_manager,
                                    std::shared_ptr<const CommandManager> command_manager,
                                    std::shared_ptr<const DeviceManager> device_manager,
                                    CompatibleRenderPassRegistry& render_pass_registry);

        virtual uint32_t getNumFramebuffers() const;
        virtual const IFramebuffer& getFramebuffer(uint32_t index) const;
        virtual IFramebuffer& getFramebuffer(uint32_t index);

        virtual const IFramebuffer& getCurrentFramebuffer() const;
        virtual IFramebuffer& getCurrentFramebuffer();

        virtual void swapFramebuffer();

        virtual void setPresentWaitSemaphores(const SemaphoreSet& semaphores);
        virtual std::shared_ptr<ManagedSemaphoreChain> addSignalImageAcquiredSemaphore();
        virtual void setSignalImageAcquiredSemaphores(const SemaphoreSet& semaphores);
    };
};
