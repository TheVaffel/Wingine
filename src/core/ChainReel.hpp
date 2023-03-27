#pragma once

#include <vector>
#include <memory>

#include "./IElementChain.hpp"

#include "../util/IndexCounter.hpp"

namespace wg::internal {
    class ChainReel {

        std::vector<std::shared_ptr<IElementChain>> element_chains;

        IndexCounter index_counter;

    public:
        ChainReel(uint32_t chain_length);

        void addChain(std::shared_ptr<IElementChain> chain);
        void removeChain(std::shared_ptr<IElementChain> chain);

        uint32_t getChainLength() const;

        void swap();
        void reset();
    };
};
