#include "./ElementChainBase.hpp"

namespace wg::internal {

    ElementChainBase::ElementChainBase(uint32_t chain_length)
        : index_counter(chain_length) { }


    uint32_t ElementChainBase::getCurrentElementIndex() const {
        return this->index_counter.getCurrentIndex();
    }

    uint32_t ElementChainBase::getElementChainLength() const {
        return this->index_counter.getNumIndices();
    }

    void ElementChainBase::swapToNextElement() {
        return this->index_counter.incrementIndex();
    }
};
