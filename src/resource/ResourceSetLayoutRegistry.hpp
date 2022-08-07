#pragma once

#include <vulkan/vulkan.hpp>

#include <map>

#include "../core/DeviceManager.hpp"

namespace wg::internal {
    class ResourceSetLayoutRegistry {
        std::map<std::vector<uint64_t>, vk::DescriptorSetLayout> layout_map;

        std::shared_ptr<const DeviceManager> device_manager;
    public:
        ResourceSetLayoutRegistry(std::shared_ptr<const DeviceManager> device_manager);

        vk::DescriptorSetLayout ensureAndGet(const std::vector<uint64_t>& resource_set_layout);

        ~ResourceSetLayoutRegistry();
    };
};
