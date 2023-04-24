#pragma once

#include "./IUniform.hpp"
#include "./IResourceChain.hpp"

namespace wg::internal {

    template<typename T>
    class IUniformChain : public IResourceChain {
    public:

        virtual void setAll(const T& value) = 0;
        virtual void setCurrent(const T& value) = 0;
        virtual IUniform<T>& getCurrentUniform() = 0;

        virtual ~IUniformChain() = default;
    };
};
