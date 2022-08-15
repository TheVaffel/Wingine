#pragma once

#include "./IElementChain.hpp"

#include "../util/IndexCounter.hpp"

namespace wg::internal {
    class ElementChainBase : virtual public IElementChain {

        IndexCounter index_counter;

    protected:

        ElementChainBase(uint32_t chain_length);

    public:
        virtual uint32_t getCurrentElementIndex() const override;
        virtual uint32_t getElementChainLength() const override;
        virtual void swapToNextElement() override;
    };
};
