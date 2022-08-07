#include "./BasicResourceSetChain.hpp"

#include "./BasicResourceSet.hpp"
#include "./StaticResourceChain.hpp"
#include "./descriptorUtil.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {
    std::shared_ptr<IResourceChain> BasicResourceSetChain::ensureChain(std::shared_ptr<IResourceChain> resource_chain) {
        return resource_chain;
    }

    std::shared_ptr<IResourceChain> BasicResourceSetChain::ensureChain(std::shared_ptr<IResource> resource) {
        return std::make_shared<StaticResourceChain>(this->resource_set_counter.getNumIndices(), resource);
    }

    IResourceSet& BasicResourceSetChain::getCurrentResourceSet() const {
        for (uint32_t i = 0; i < this->resource_chains.size(); i++) {
            fl_assert_eq(this->resource_chains[i]->getCurrentIndex(),
                         this->resource_set_counter.getCurrentIndex());
        }

        return *this->resource_sets[this->resource_set_counter.getCurrentIndex()];
    }

    IResourceSet& BasicResourceSetChain::getResourceSetAt(uint32_t index) {
        return *this->resource_sets[index];
    }

    uint32_t BasicResourceSetChain::getNumResources() const {
        return this->resource_sets.size();
    }

    uint32_t BasicResourceSetChain::getCurrentResourceIndex() const {
        return this->resource_set_counter.getCurrentIndex();
    }

    void BasicResourceSetChain::swap() {
        this->resource_set_counter.incrementIndex();
    }

    void BasicResourceSetChain::writeToDescriptorSets(
        uint32_t chain_length,
        const std::vector<std::shared_ptr<IResourceChain>>& resource_chains) {

        for (uint32_t i = 0; i < chain_length; i++) {
            std::vector<IResource*> tmp_resources(resource_chains.size());

            for (uint32_t j = 0; j < resource_chains.size(); j++) {
                tmp_resources[j] = &resource_chains[j]->getResourceAt(i);
            }

            descriptorUtil::writeToDescriptorSet(tmp_resources,
                                                 this->resource_sets[i]->getDescriptorSet(),
                                                 this->device_manager->getDevice());
        }
    }

    void BasicResourceSetChain::setChains(const std::vector<std::shared_ptr<IResourceChain>>& resource_chains) {
        fl_assert_gt(resource_chains.size(), 0u);

        uint32_t chain_length = resource_chains[0]->getNumResources();
        fl_assert_eq(chain_length, this->resource_sets.size());

        this->resource_chains.resize(resource_chains.size());

        for (uint32_t i = 0; i < resource_chains.size(); i++) {
            this->resource_chains[i] = resource_chains[i];
            fl_assert_eq(resource_chains[i]->getNumResources(), chain_length);
        }

        this->writeToDescriptorSets(chain_length, resource_chains);
    }
};
