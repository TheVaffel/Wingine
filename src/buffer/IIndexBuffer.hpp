#pragma once

#include "./IBuffer.hpp"

namespace wg::internal {
    class IIndexBuffer : public virtual IBuffer {
    public:

        virtual void set(const uint32_t* data, uint32_t first_index, uint32_t num_indices) = 0;
        virtual ~IIndexBuffer() = default;

        uint32_t getNumIndices() const;
    };
};
