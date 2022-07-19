#include "./copyBuffer.hpp"

#include <flawed_assert.hpp>

namespace wg::internal::copyBuffer {
    void recordCopyBuffer(const vk::CommandBuffer& command,
                          const IBuffer& src,
                          const IBuffer& dst,
                          uint64_t copy_size,
                          uint64_t src_offset,
                          uint64_t dst_offset) {

        vk::CommandBufferBeginInfo begin_info;
        command.begin(begin_info);

        if (copy_size == VK_WHOLE_SIZE) {
            copy_size = std::min(src.getByteSize(), dst.getByteSize());
        } else {
            fl_assert_le(copy_size + dst_offset, src.getByteSize());
            fl_assert_le(copy_size + src_offset, dst.getByteSize());
        }



        vk::BufferCopy copy_info;
        copy_info.setSrcOffset(src_offset)
            .setDstOffset(dst_offset)
            .setSize(copy_size);
        command.copyBuffer(src.getBuffer(),
                           dst.getBuffer(),
                           1,
                           &copy_info);

        command.end();
    }
};
