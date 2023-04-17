#pragma once

#include "./IResourceChain.hpp"

#include <memory>

namespace wg::internal {
    struct ResourceBinding {
        uint32_t binding_index;
        std::shared_ptr<IResourceChain> resource;

        ResourceBinding(uint32_t binding_index, std::shared_ptr<IResourceChain> resource);
    };
};
