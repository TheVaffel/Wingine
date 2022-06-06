#pragma once

#include "./BasicUniformChain.hpp"

namespace wg::internal {
    template<typename T>
    BasicUniformChain<T>::BasicUniformChain(uint32_t count,
                                            std::shared_ptr<const DeviceManager> device_manager)
        : uniform_index(count) {
        for (uint32_t i = 0; i < count; i++) {
            this->uniforms.push_back(std::make_shared<BasicUniform<T>>(device_manager));
        }
    }

    template<typename T>
    void BasicUniformChain<T>::setAllUniforms(const T& value) {
        for (uint32_t i = 0; i < this->uniforms.size(); i++) {
            this->uniforms[i]->set(value);
        }
    }

    /* template<typename T>
    void BasicUniformChain<T>::setNextUniformAndSwap(const T& value) {
        this->uniform_index.incrementIndex();
        this->uniforms[this->uniform_index.getCurrentIndex()]->set(value);
        } */
    template<typename T>
    void BasicUniformChain<T>::setCurrentUniform(const T& value) {
        this->uniforms[this->uniform_index.getCurrentIndex()]->set(value);
    }

    template<typename T>
    void BasicUniformChain<T>::swap() {
        this->uniform_index.incrementIndex();
    }

    template<typename T>
    IUniform<T>& BasicUniformChain<T>::getCurrentUniform() {
        return *this->uniforms[this->uniform_index.getCurrentIndex()];
    }

    template<typename T>
    uint32_t BasicUniformChain<T>::getCurrentIndex() const {
        return this->uniform_index.getCurrentIndex();
    }

    template<typename T>
    uint32_t BasicUniformChain<T>::getNumResources() const {
        return this->uniforms.size();
    }

    template<typename T>
    IResource& BasicUniformChain<T>::getResourceAt(uint32_t index) {
        return *this->uniforms[index];
    }
};
