#pragma once

#include "./IResourceChain.hpp"
#include "./IResourceSet.hpp"

namespace wg::internal {
    class IResourceSetChain {
    public:
        virtual void set(const std::vector<std::shared_ptr<IResourceChain>>& resource_chains) = 0;
        virtual IResourceSet& getCurrentResourceSet() const = 0;
        virtual IResourceSet& getResourceSetAt(uint32_t index) = 0;
        virtual uint32_t getNumResources() const = 0;
        virtual uint32_t getCurrentResourceIndex() const = 0;
        virtual void swap() = 0;
    };
};