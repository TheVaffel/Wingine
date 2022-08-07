#pragma once

#include "./IResourceSetChain.hpp"

#include "../util/IndexCounter.hpp"
#include "../DeviceManager.hpp"

namespace wg::internal {
    class BasicResourceSetChain : public IResourceSetChain {
        IndexCounter resource_set_counter;

        std::vector<std::shared_ptr<IResourceSet>> resource_sets;
        std::vector<std::shared_ptr<IResourceChain>> resource_chains;

        std::shared_ptr<const DeviceManager> device_manager;

        void writeToDescriptorSets(
            uint32_t chain_length,
            const std::vector<std::shared_ptr<IResourceChain>>& resource_chains);


        std::shared_ptr<IResourceChain> ensureChain(std::shared_ptr<IResourceChain> resource_chain);
        std::shared_ptr<IResourceChain> ensureChain(std::shared_ptr<IResource> resource);

        template<typename T, typename... Ts>
        void ensureOnlyChains(std::vector<std::shared_ptr<IResourceChain>>& results, T resource, Ts... rest_resources);

        void setChains(const std::vector<std::shared_ptr<IResourceChain>>& resource_chains);

    public:
        template<typename... Ts>
        BasicResourceSetChain(uint32_t chain_length,
                              const vk::DescriptorSetLayout& layout,
                              const vk::DescriptorPool& pool,
                              std::shared_ptr<const DeviceManager> device_manager,
                              Ts... resources);


        virtual IResourceSet& getCurrentResourceSet() const override;
        virtual IResourceSet& getResourceSetAt(uint32_t index) override;
        virtual uint32_t getNumResources() const override;
        virtual uint32_t getCurrentResourceIndex() const override;

        virtual void swap() override;
    };
};

#include "./BasicResourceSetChain.impl.hpp"
