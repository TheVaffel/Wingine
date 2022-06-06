#pragma once

#include "./Wingine.hpp"

#include "./resource/BasicUniform.hpp"
#include "./resource/BasicUniformChain.hpp"

namespace wg {
    template<typename T>
    UniformPtr<T> Wingine::createUniform() {
        return std::make_shared<internal::BasicUniform<T>>(this->device_manager);
    }

    template<typename T>
    UniformChainPtr<T> Wingine::createUniformChain() {
        return std::make_shared<internal::BasicUniformChain<T>>(this->getNumFramebuffers(),
                                                                this->device_manager);
    }
};
