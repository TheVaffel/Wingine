#pragma once

#include "./IResourceChain.hpp"
#include "./IResourceSet.hpp"

#include "../core/IElementChain.hpp"

namespace wg::internal {
    class IResourceSetChain : virtual public IElementChain {
    public:
        virtual IResourceSet& getCurrentResourceSet() const = 0;
        virtual IResourceSet& getResourceSetAt(uint32_t index) = 0;
    };
};
