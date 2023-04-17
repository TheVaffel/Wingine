#pragma once

#include "./BasicResourceSet.hpp"
#include "./BasicResourceSetChain.hpp"

namespace wg::internal {

    template<typename... Ts>
    BasicResourceSetChain::BasicResourceSetChain(uint32_t chain_length,
                                                 const std::vector<ResourceBinding>& bindings,
                                                 const vk::DescriptorSetLayout& layout,
                                                 const vk::DescriptorPool& pool,
                                                 std::shared_ptr<const DeviceManager> device_manager)
        : ElementChainBase(chain_length), device_manager(device_manager) {
        for (uint32_t i = 0; i < chain_length; i++) {
            this->resource_sets.push_back(std::make_shared<BasicResourceSet>(layout, pool, device_manager));
        }

        std::vector<std::shared_ptr<IResourceChain>> chains;
        for (const ResourceBinding& binding : bindings) {
            chains.push_back(binding.resource);
        }

        this->setChains(chains);

        this->layout = layout;
    }
};
