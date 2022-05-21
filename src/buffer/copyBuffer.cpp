#include "./copyBuffer.hpp"

namespace wg::internal::copyBuffer {
    void recordCopyBuffer(const vk::CommandBuffer& command,
                          const IBuffer& src,
                          const IBuffer& dst) {

        vk::CommandBufferBeginInfo begin_info;
        command.begin(begin_info);

        vk::BufferCopy copy_info;
        copy_info.setSrcOffset(0)
            .setDstOffset(0)
            .setSize(std::min(src.getByteSize(), dst.getByteSize()));
        command.copyBuffer(src.getBuffer(),
                           dst.getBuffer(),
                           1,
                           &copy_info);

        command.end();
    }
};
