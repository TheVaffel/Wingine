#pragma once

#include <cstdint>

namespace wg::internal {
    class IElementChain {
    public:
        virtual uint32_t getCurrentElementIndex() const = 0;
        virtual uint32_t getElementChainLength() const = 0;
        virtual void swapToNextElement() = 0;
    };
};
