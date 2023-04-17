#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class WrappedDSLCI {
        vk::DescriptorSetLayoutCreateInfo create_info;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

    public:
        WrappedDSLCI(std::vector<vk::DescriptorSetLayoutBinding> bindings);

        WrappedDSLCI(const WrappedDSLCI&);

        WrappedDSLCI& operator=(const WrappedDSLCI&);

        vk::DescriptorSetLayoutCreateInfo getCreateInfo() const;
    };
};
