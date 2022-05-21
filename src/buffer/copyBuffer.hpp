#pragma once

#include "./IBuffer.hpp"

namespace wg::internal::copyBuffer {
    void recordCopyBuffer(const vk::CommandBuffer& command,
                          const IBuffer& src,
                          const IBuffer& dst);
};
