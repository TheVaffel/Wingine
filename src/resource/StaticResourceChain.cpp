#include "./StaticResourceChain.hpp"

namespace wg::internal {
    StaticResourceChain::StaticResourceChain(uint32_t num_resources,
                                             std::shared_ptr<IResource> resource)
        : ElementChainBase(num_resources),
          resource(resource) { }

    IResource& StaticResourceChain::getResourceAt(uint32_t index) {
        return *this->resource;
    }
};
