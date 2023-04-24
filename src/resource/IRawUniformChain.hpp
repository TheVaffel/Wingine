#pragma once

#include "./IRawUniform.hpp"
#include "./IResourceChain.hpp"

namespace wg::internal {

    class IRawUniformChain : public IResourceChain {
    public:

        virtual void setAll(const void* ptr) = 0;
        virtual void setCurrent(const void* ptr) = 0;
        virtual IRawUniform& getCurrentUniform() = 0;

        virtual ~IRawUniformChain() = default;
    };
};
