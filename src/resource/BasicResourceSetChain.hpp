#pragma once

#include "./IResourceSetChain.hpp"

#include "../util/IndexCounter.hpp"
#include "../core/DeviceManager.hpp"
#include "../core/ElementChainBase.hpp"

#include "./ResourceBinding.hpp"

namespace wg::internal {
    class BasicResourceSetChain : public IResourceSetChain, public ElementChainBase {
        std::vector<std::shared_ptr<IResourceSet>> resource_sets;
        std::vector<std::shared_ptr<IResourceChain>> resource_chains;

        std::shared_ptr<const DeviceManager> device_manager;

        vk::DescriptorSetLayout layout;

        void writeToDescriptorSets(
            uint32_t chain_length,
            const std::vector<std::shared_ptr<IResourceChain>>& resource_chains);


        std::shared_ptr<IResourceChain> ensureChain(std::shared_ptr<IResourceChain> resource_chain);
        std::shared_ptr<IResourceChain> ensureChain(std::shared_ptr<IResource> resource);

        template<typename T, typename... Ts>
        void ensureOnlyChains(std::vector<std::shared_ptr<IResourceChain>>& results, T resource, Ts... rest_resources);

        void setChains(const std::vector<std::shared_ptr<IResourceChain>>& resource_chains);

    public:

        BasicResourceSetChain(uint32_t chain_length,
                              const std::vector<ResourceBinding>& bindings,
                              const vk::DescriptorSetLayout& layout,
                              const vk::DescriptorPool& pool,
                              std::shared_ptr<const DeviceManager> device_manager);


        virtual IResourceSet& getCurrentResourceSet() const override;
        virtual IResourceSet& getResourceSetAt(uint32_t index) override;

        virtual ~BasicResourceSetChain();
    };
};

#include "./BasicResourceSetChain.impl.hpp"
