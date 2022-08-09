#pragma once

#include "./BasicUniformChain.hpp"

namespace wg::internal {
    template<typename T>
    BasicUniformChain<T>::BasicUniformChain(uint32_t count,
                                            std::shared_ptr<const DeviceManager> device_manager)
        : ElementChainBase(count) {
        for (uint32_t i = 0; i < count; i++) {
            this->uniforms.push_back(std::make_shared<BasicUniform<T>>(device_manager));
        }
    }

    template<typename T>
    void BasicUniformChain<T>::setAll(const T& value) {
        for (uint32_t i = 0; i < this->uniforms.size(); i++) {
            this->uniforms[i]->set(value);
        }
    }

    template<typename T>
    void BasicUniformChain<T>::setCurrent(const T& value) {
        this->uniforms[this->getCurrentElementIndex()]->set(value);
    }

    template<typename T>
    IUniform<T>& BasicUniformChain<T>::getCurrentUniform() {
        return *this->uniforms[this->getCurrentElementIndex()];
    }

    template<typename T>
    IResource& BasicUniformChain<T>::getResourceAt(uint32_t index) {
        return *this->uniforms[index];
    }
};
