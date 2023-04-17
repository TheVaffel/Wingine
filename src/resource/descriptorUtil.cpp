#include "./descriptorUtil.hpp"

#include <iostream>

#include <flawed_assert.hpp>

namespace wg::internal::descriptorUtil {

    std::map<uint32_t, vk::DescriptorSetLayout> createDescriptorSetLayoutFromInfos(const std::map<uint32_t, WrappedDSLCI>& infos,
                                                                                   const vk::Device& device) {

        std::map<uint32_t, vk::DescriptorSetLayout> layouts;
        for (auto& set_and_info : infos) {
            // Ensure the set numbers are consecutive
            fl_assert_lt(set_and_info.first, infos.size());

            layouts[set_and_info.first] = device.createDescriptorSetLayout(set_and_info.second.getCreateInfo());
        }

        return layouts;
    }

    vk::DescriptorSetLayout createDescriptorSetLayoutFromBindings(const std::span<const vk::DescriptorSetLayoutBinding>& bindings,
                                                                  const vk::Device& device) {
        vk::DescriptorSetLayoutCreateInfo dlc;
        dlc.setBindingCount((uint32_t) bindings.size())
            .setPBindings(bindings.data());

        vk::DescriptorSetLayout layout =
            device.createDescriptorSetLayout(dlc);

        return layout;
    }

    vk::DescriptorSet allocateDescriptorSet(const vk::DescriptorPool& pool,
                                            const vk::DescriptorSetLayout& layout,
                                            const vk::Device& device) {

        vk::DescriptorSetAllocateInfo dsai;
        dsai.setDescriptorPool(pool)
            .setDescriptorSetCount(1)
            .setPSetLayouts(&layout);

        return device.allocateDescriptorSets(dsai)[0];
    }

    void writeToDescriptorSet(const std::vector<IResource*>& resources,
                              const vk::DescriptorSet descriptor_set,
                              const vk::Device& device) {
        std::vector<vk::WriteDescriptorSet> writes(resources.size());

        std::vector<std::unique_ptr<internal::IResourceWriteAuxillaryData>> aux_data(resources.size());
        for(unsigned int i = 0; i < resources.size(); i++) {
            writes[i].setDstSet(descriptor_set)
                .setDstBinding(i);
            aux_data[i] = resources[i]->writeDescriptorUpdate(writes[i]);
        }

        device.updateDescriptorSets(writes, {});
    }
};
