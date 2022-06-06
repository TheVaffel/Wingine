#pragma once

#include <vulkan/vulkan.hpp>

#include "./IResource.hpp"

namespace wg::internal::descriptorUtil {
    vk::DescriptorSetLayout createDescriptorSetLayout(const std::vector<uint64_t>& flags,
                                                      const vk::Device& device);

    vk::DescriptorSet allocateDescriptorSet(const vk::DescriptorPool& pool,
                                            const vk::DescriptorSetLayout& layout,
                                            const vk::Device& device);

    void writeToDescriptorSet(const std::vector<IResource*>& resources,
                              const vk::DescriptorSet descriptor_set,
                              const vk::Device& device);
};