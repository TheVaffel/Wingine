#pragma once

#include <cstdint>

#include "./IResource.hpp"

namespace wg::internal {
    class IResourceChain {
    public:
        virtual void swap() = 0;
        virtual uint32_t getCurrentIndex() const = 0;
        virtual uint32_t getNumResources() const = 0;

        virtual IResource& getResourceAt(uint32_t index) = 0;

        virtual ~IResourceChain() = default;
    };
};
