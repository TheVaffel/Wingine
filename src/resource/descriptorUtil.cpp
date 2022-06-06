#include "./descriptorUtil.hpp"

namespace wg::internal::descriptorUtil {

    vk::DescriptorSetLayout createDescriptorSetLayout(const std::vector<uint64_t>& flags,
                                                      const vk::Device& device) {
        std::vector<vk::DescriptorSetLayoutBinding> lbs(flags.size());

        int i = 0;
        for(uint64_t flag : flags) {
            lbs[i].setBinding(i)
                .setDescriptorCount(1)
                .setStageFlags(vk::ShaderStageFlagBits(flag >> 32))
                .setDescriptorType(vk::DescriptorType(flag & ((1LL << 32) - 1)));
            i++;
        }

        vk::DescriptorSetLayoutCreateInfo dlc;
        dlc.setBindingCount((uint32_t)lbs.size())
            .setPBindings(lbs.data());

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
