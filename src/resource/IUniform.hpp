#pragma once

#include "./IResource.hpp"

namespace wg::internal {
    template<typename T>
    class IUniform : public virtual IResource {
    public:
        virtual void set(const T& val) = 0;

        virtual ~IUniform() = default;
    };
};
