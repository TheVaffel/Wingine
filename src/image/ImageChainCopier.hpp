#pragma once

#include "./IImage.hpp"
#include "./CopyImageAuxillaryData.hpp"

#include "../sync/ISynchronizedQueueOperation.hpp"

#include "../CommandManager.hpp"

#include "../framebuffer/IFramebufferChain.hpp"
#include "./ITextureChain.hpp"

namespace wg::internal {
    class ImageChainCopier : public ISynchronizedQueueOperation {

        IndexCounter image_index;

        SignalAndWaitSemaphores semaphores;

        std::vector<Command> commands;
        std::vector<CopyImageAuxillaryData> auxillary_data;

        std::shared_ptr<const DeviceManager> device_manager;
        std::shared_ptr<const CommandManager> command_manager;
        vk::Queue queue;

        void recordCopies(const std::vector<IImage*>& srcs,
                          const std::vector<IImage*>& dsts);

    public:
        ImageChainCopier(uint32_t count,
                         const vk::Queue& queue,
                         std::shared_ptr<const CommandManager> command_manager,
                         std::shared_ptr<const DeviceManager> device_manager);

        void recordCopies(std::shared_ptr<IFramebufferChain> framebuffer_chain,
                          std::shared_ptr<ITextureChain> texture_chain);

        virtual SignalAndWaitSemaphores& getSemaphores() override;

        void runCopyAndSwap();

        ~ImageChainCopier();
    };
};
