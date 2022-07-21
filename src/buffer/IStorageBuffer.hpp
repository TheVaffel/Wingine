#pragma once

#include "../resource/IResource.hpp"
#include "./IBuffer.hpp"

namespace wg::internal {
    template <typename T>
    class IStorageBuffer : public virtual IResource, public virtual IBuffer {
    public:
        virtual void set(const T* data, uint32_t first_element, uint32_t element_count) = 0;
    };
};
