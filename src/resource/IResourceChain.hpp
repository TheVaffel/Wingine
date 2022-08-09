#pragma once

#include <cstdint>

#include "./IResource.hpp"
#include "../core/IElementChain.hpp"

namespace wg::internal {
    class IResourceChain : virtual public IElementChain {
    public:
        virtual IResource& getResourceAt(uint32_t index) = 0;

        virtual ~IResourceChain() = default;
    };
};
