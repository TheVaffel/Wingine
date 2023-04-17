#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class WrappedPLCI {
        vk::PipelineLayoutCreateInfo info;
        std::vector<vk::DescriptorSetLayout> layouts;
    public:

        WrappedPLCI(vk::PipelineLayoutCreateInfo info, const std::vector<vk::DescriptorSetLayout>& layouts);

        WrappedPLCI(const WrappedPLCI&);
        WrappedPLCI& operator=(const WrappedPLCI&);

        vk::PipelineLayoutCreateInfo getCreateInfo() const;

        const std::vector<vk::DescriptorSetLayout>& getLayouts() const;
    };
};
