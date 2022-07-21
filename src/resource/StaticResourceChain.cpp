#include "./StaticResourceChain.hpp"

namespace wg::internal {


    StaticResourceChain::StaticResourceChain(uint32_t num_resources,
                                             std::shared_ptr<IResource> resource)
        : index_counter(num_resources),
          resource(resource) { }

    void StaticResourceChain::swap() {
        this->index_counter.incrementIndex();
    }

    uint32_t StaticResourceChain::getCurrentIndex() const {
        return this->index_counter.getCurrentIndex();
    }
    uint32_t StaticResourceChain::getNumResources() const {
        return this->index_counter.getNumIndices();
    }

    IResource& StaticResourceChain::getResourceAt(uint32_t index) {
        return *this->resource;
    }
};
