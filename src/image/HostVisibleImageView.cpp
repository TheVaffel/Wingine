#include "./HostVisibleImageView.hpp"

#include "../buffer/BasicBuffer.hpp"
#include "../memory/memoryUtil.hpp"
#include "../sync/fenceUtil.hpp"

#include "./BasicImage.hpp"

#include <iostream>

namespace wg::internal {

    using pixel_type = uint32_t;

    HostVisibleImageView::HostVisibleImageView(uint32_t num_buffers,
                                               const vk::Extent2D& dimensions,
                                               std::shared_ptr<const DeviceManager> device_manager)
        : dimensions(dimensions),
          buffers(num_buffers),
          debug_image(dimensions, BasicImageSetup::createHostAccessibleColorImageSetup(), device_manager),
          current_buffer_counter(num_buffers),
          device_manager(device_manager) {

        this->initBuffers(dimensions);
    }

    void HostVisibleImageView::initBuffers(const vk::Extent2D& dimensions) {
        for (uint32_t i = 0; i < this->buffers.size(); i++) {
            this->buffers[i] =
                BasicBuffer::createHostVisibleBuffer(dimensions.width * dimensions.height * sizeof(pixel_type),
                                                     this->device_manager);
        }
    }

    IBuffer& HostVisibleImageView::getCurrentBuffer() {
        return *this->buffers[this->current_buffer_counter.getCurrentIndex()];
    }

    IBuffer& HostVisibleImageView::getBuffer(uint32_t index) {
        return *this->buffers[index];
    }

    IImage& HostVisibleImageView::getImage() {
        return this->debug_image;
    }

    uint32_t HostVisibleImageView::getNumBuffers() const {
        return this->buffers.size();
    }

    vk::Extent2D HostVisibleImageView::getDimensions() const {
        return this->dimensions;
    }

    void  HostVisibleImageView::setReadyForCopyFence(const vk::Fence& fence) {
        this->ready_for_copy_fence = fence;
    }

    void HostVisibleImageView::swapBuffers() {
        current_buffer_counter.incrementIndex();
    }

    uint32_t HostVisibleImageView::getByteStride() const {
        // We don't add any padding at the end of each row when copying to buffer
        return this->dimensions.width * sizeof(pixel_type);
    }

    void HostVisibleImageView::copyImageToHost(pixel_type* dst) {
        fenceUtil::awaitFence(this->ready_for_copy_fence,
                              this->device_manager->getDevice());



        uint32_t* src =
            memoryUtil::mapMemory<uint32_t>(this->buffers[current_buffer_counter.getCurrentIndex()]->getMemory(),
                                            this->device_manager->getDevice());

        uint32_t num_bytes_to_copy = this->getByteStride() * this->getDimensions().height;

        memcpy(dst, src, num_bytes_to_copy);

        memoryUtil::unmapMemory(this->buffers[current_buffer_counter.getCurrentIndex()]->getMemory(),
                                this->device_manager->getDevice());
    }
};
