#include "./ResourceBinding.hpp"

namespace wg::internal {
    ResourceBinding::ResourceBinding(uint32_t binding_index,
                                     std::shared_ptr<IResourceChain> resource) {
        this->resource = resource;
        this->binding_index = binding_index;
    }
};
