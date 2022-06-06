#include "./ResourceSetLayoutRegistry.hpp"

#include "./descriptorUtil.hpp"

namespace wg::internal {
    ResourceSetLayoutRegistry::ResourceSetLayoutRegistry(std::shared_ptr<const DeviceManager> device_manager)
        : device_manager(device_manager) { }

    vk::DescriptorSetLayout
    ResourceSetLayoutRegistry::ensureAndGet(const std::vector<uint64_t>& resource_set_layout) {
        if (this->layout_map.find(resource_set_layout) == this->layout_map.end()) {
            this->layout_map[resource_set_layout] =
                descriptorUtil::createDescriptorSetLayout(resource_set_layout, this->device_manager->getDevice());
        }

        return this->layout_map[resource_set_layout];
    }

    ResourceSetLayoutRegistry::~ResourceSetLayoutRegistry() {
        for (auto it : this->layout_map) {
            this->device_manager->getDevice().destroy(it.second);
        }
    }
};
