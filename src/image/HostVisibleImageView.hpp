#pragma once

#include "../buffer/IBuffer.hpp"

#include "../DeviceManager.hpp"
#include "../util/IndexCounter.hpp"

#include "./IImage.hpp"

namespace wg::internal {
    class HostVisibleImageView {
        vk::Extent2D dimensions;
        std::vector<std::unique_ptr<IBuffer>> buffers;
        std::shared_ptr<IImage> debug_image;

        IndexCounter current_buffer_counter;

        std::shared_ptr<const DeviceManager> device_manager;

        vk::Fence ready_for_copy_fence;

        void initBuffers(const vk::Extent2D& dimensions);

    public:
        HostVisibleImageView(uint32_t num_buffers,
                             const vk::Extent2D& dimensions,
                             std::shared_ptr<const DeviceManager> device_manager);

        IBuffer& getBuffer(uint32_t index);
        IBuffer& getCurrentBuffer();

        IImage& getImage();

        vk::Extent2D getDimensions() const;
        uint32_t getNumBuffers() const;

        void setReadyForCopyFence(const vk::Fence& fence);

        void swapBuffers();

        /*
         * Gets the number of bytes between two pixels at the same x-coordinate, one row apart
         */
        uint32_t getByteStride() const;

        void copyImageToHost(uint32_t* dst);
    };
};
