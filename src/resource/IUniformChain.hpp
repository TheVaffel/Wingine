#pragma once

#include "./IUniform.hpp"
#include "./IResourceChain.hpp"

#include "../sync/ISynchronizedQueueOperation.hpp"

#include <concepts>

namespace wg::internal {

    template<typename T>
    class IUniformChain : public IResourceChain {
    public:

        virtual void setAllUniforms(const T& value) = 0;
        // virtual void setNextUniformAndSwap(const T& value) = 0;
        virtual void setCurrentUniform(const T& value) = 0;
        virtual IUniform<T>& getCurrentUniform() = 0;

        virtual ~IUniformChain() = default;
    };
};
