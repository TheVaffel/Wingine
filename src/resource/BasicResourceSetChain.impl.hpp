#pragma once

#include "./BasicResourceSet.hpp"
#include "./BasicResourceSetChain.hpp"

namespace wg::internal {

    template<typename... Ts>
    BasicResourceSetChain::BasicResourceSetChain(uint32_t chain_length,
                                                 const vk::DescriptorSetLayout& layout,
                                                 const vk::DescriptorPool& pool,
                                                 std::shared_ptr<const DeviceManager> device_manager,
                                                 Ts... resources)
        : resource_set_counter(chain_length), device_manager(device_manager) {
        for (uint32_t i = 0; i < chain_length; i++) {
            this->resource_sets.push_back(std::make_shared<BasicResourceSet>(layout, pool, device_manager));
        }

        std::vector<std::shared_ptr<IResourceChain>> chains;
        this->ensureOnlyChains(chains, resources...);
        this->setChains(chains);
    }

    template<typename T, typename... Ts>
    void BasicResourceSetChain::ensureOnlyChains(std::vector<std::shared_ptr<IResourceChain>>& result,
                                                 T resource,
                                                 Ts... resources) {
        result.push_back(this->ensureChain(resource));
        if constexpr (sizeof...(Ts) > 0) {
            ensureOnlyChains(result, resources...);
        }
    }
};
