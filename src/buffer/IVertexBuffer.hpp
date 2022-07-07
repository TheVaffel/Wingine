#pragma once

#include "./IBuffer.hpp"

namespace wg::internal {
    template<typename T>
    class IVertexBuffer : public virtual IBuffer {
    public:

        virtual void set(const T* data, uint32_t first_element, uint32_t element_count) = 0;

        virtual ~IVertexBuffer() = default;
    };
};
