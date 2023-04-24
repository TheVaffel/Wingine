#include "./BasicRawUniformChain.hpp"

namespace wg::internal {

    BasicRawUniformChain::BasicRawUniformChain(uint32_t count,
                                            uint32_t byte_size,
                                            std::shared_ptr<const DeviceManager> device_manager)
        : ElementChainBase(count) {

        for (uint32_t i = 0; i < count; i++) {
            this->uniforms.push_back(std::make_shared<BasicRawUniform>(byte_size, device_manager));
        }
    }

    void BasicRawUniformChain::setAll(const void* ptr) {
        for (uint32_t i = 0; i < this->uniforms.size(); i++) {
            this->uniforms[i]->set(ptr);
        }
    }

    void BasicRawUniformChain::setCurrent(const void* ptr) {
        this->uniforms[this->getCurrentElementIndex()]->set(ptr);
    }
    IRawUniform& BasicRawUniformChain::getCurrentUniform() {
        return *this->uniforms[this->getCurrentElementIndex()];
    }

    IResource& BasicRawUniformChain::getResourceAt(uint32_t index) {
        return *this->uniforms[index];
    }

};
