#include "./BasicResourceSet.hpp"

#include "./descriptorUtil.hpp"

namespace wg::internal {

    BasicResourceSet::BasicResourceSet(vk::DescriptorSetLayout layout,
                                       vk::DescriptorPool descriptor_pool,
                                       std::shared_ptr<const DeviceManager> device_manager)
        : device_manager(device_manager) {

        this->descriptor_set = descriptorUtil::allocateDescriptorSet(descriptor_pool,
                                                                     layout,
                                                                     device_manager->getDevice());
    }

    vk::DescriptorSet BasicResourceSet::getDescriptorSet() const {
        return this->descriptor_set;
    }

    void BasicResourceSet::set(const std::vector<std::shared_ptr<IResource>>& resources) {
        std::vector<vk::WriteDescriptorSet> writes(resources.size());

        std::vector<std::unique_ptr<internal::IResourceWriteAuxillaryData>> aux_data(resources.size());
        for(unsigned int i = 0; i < resources.size(); i++) {
            writes[i].setDstSet(this->descriptor_set)
                .setDstBinding(i);
            aux_data[i] = resources[i]->writeDescriptorUpdate(writes[i]);
        }

        this->device_manager->getDevice().updateDescriptorSets(writes, {});
    }
}
