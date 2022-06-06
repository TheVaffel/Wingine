#pragma once

#include "./IResourceSet.hpp"
#include "../DeviceManager.hpp"

namespace wg::internal {
    class BasicResourceSet : public IResourceSet {
        vk::DescriptorSet descriptor_set;

        std::shared_ptr<const DeviceManager> device_manager;

    public:
        BasicResourceSet(vk::DescriptorSetLayout layout,
                         vk::DescriptorPool descriptor_pool,
                         std::shared_ptr<const DeviceManager> device_manager);

        virtual vk::DescriptorSet getDescriptorSet() const override;
        virtual void set(const std::vector<std::shared_ptr<IResource>>& resources) override;
    };
};
