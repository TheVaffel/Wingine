#pragma once

#include <vulkan/vulkan.hpp>

#include "./IResource.hpp"
#include "./WrappedDSLCI.hpp"

#include <span>
#include <map>

namespace wg::internal::descriptorUtil {

    vk::DescriptorSetLayout createDescriptorSetLayoutFromBindings(const std::span<const vk::DescriptorSetLayoutBinding>& bindings,
                                                                  const vk::Device& device);

    std::map<uint32_t, vk::DescriptorSetLayout> createDescriptorSetLayoutFromInfos(const std::map<uint32_t, WrappedDSLCI>& infos,
                                                                                   const vk::Device& device);
    vk::DescriptorSet allocateDescriptorSet(const vk::DescriptorPool& pool,
                                            const vk::DescriptorSetLayout& layout,
                                            const vk::Device& device);

    void writeToDescriptorSet(const std::vector<IResource*>& resources,
                              const vk::DescriptorSet descriptor_set,
                              const vk::Device& device);
};
