#pragma once

#include "./IResource.hpp"

#include <cstdint>

namespace wg::internal {
    class IRawUniform : public IResource {
    public:
        virtual void set(const void* ptr) = 0;
        virtual uint32_t getByteSize() const = 0;

        virtual ~IRawUniform() = default;
    };
};
