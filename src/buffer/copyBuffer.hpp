#pragma once

#include "./IBuffer.hpp"

namespace wg::internal::copyBuffer {
    void recordCopyBuffer(const vk::CommandBuffer& command,
                          const IBuffer& src,
                          const IBuffer& dst,
                          uint64_t copy_size = VK_WHOLE_SIZE,
                          uint64_t src_offset = 0,
                          uint64_t dst_offset = 0

        );
};
