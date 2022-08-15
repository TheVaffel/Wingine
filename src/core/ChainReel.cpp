#include "./ChainReel.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {

    ChainReel::ChainReel(uint32_t chain_length)
        : index_counter(chain_length) { }

    void ChainReel::addChain(std::shared_ptr<IElementChain> chain) {
        fl_assert_eq(this->getChainLength(), chain->getElementChainLength());
        fl_assert_eq(this->index_counter.getCurrentIndex(), chain->getCurrentElementIndex());

        this->element_chains.push_back(chain);
    }

    void ChainReel::removeChain(std::shared_ptr<IElementChain> chain) {
        for (uint32_t i = 0; i < this->element_chains.size(); i++) {
            if (chain.get() == element_chains[i].get()) {
                this->element_chains[i] =
                    this->element_chains[this->element_chains.size() - 1];
                this->element_chains.pop_back();
                return;
            }
        }

        throw std::runtime_error("[ChainReel] Could not find chain to delete");
    }

    uint32_t ChainReel::getChainLength() const {
        return this->index_counter.getNumIndices();
    }

    void ChainReel::swap() {
        this->index_counter.incrementIndex();

        for (const std::shared_ptr<IElementChain>& chain : this->element_chains) {
            chain->swapToNextElement();

            fl_assert_eq(chain->getCurrentElementIndex(), this->index_counter.getCurrentIndex());
        }
    }
};
