#include "./util.hpp"

#include <iostream>
#include <span>

namespace wg::spirv::util {
    vk::ShaderStageFlagBits executionModelToShaderStage(ExecutionModel model) {
        switch (model) {
        case ExecutionModel::Vertex:
            return vk::ShaderStageFlagBits::eVertex;
        case ExecutionModel::Fragment:
            return vk::ShaderStageFlagBits::eFragment;
        case ExecutionModel::GLCompute:
            return vk::ShaderStageFlagBits::eCompute;
        default:
            throw std::runtime_error("Execution model not recognized");
        }
    }

    vk::DescriptorType getDescriptorType(Variable var, ParseResult& parse_result) {
        switch (var.storage_class) {
        case StorageClass::Uniform:
            return vk::DescriptorType::eUniformBuffer;
        case StorageClass::StorageBuffer:
            return vk::DescriptorType::eStorageBuffer;
        case StorageClass::UniformConstant:
            {
                uint32_t pointer_type_id = var.type_identifier;
                uint32_t base_type_id = parse_result.pointer_types[pointer_type_id].type_identifier;


                BaseType base_type = parse_result.base_types[base_type_id].base_type;

                switch (base_type) {
                case BaseType::SampledImage:
                    return vk::DescriptorType::eCombinedImageSampler;
                case BaseType::Image:
                    return vk::DescriptorType::eStorageImage;
                default:
                    return vk::DescriptorType::eUniformBuffer;
                }
            }
            break;
        default:
            throw std::runtime_error("[Spirv] unknown storage class");
        }
    }

    vk::DescriptorSetLayout createDescriptorSetLayoutFromBindings(const std::span<const vk::DescriptorSetLayoutBinding>& bindings,
                                                                  const vk::Device& device) {
        vk::DescriptorSetLayoutCreateInfo dlc;
        dlc.setBindingCount(bindings.size())
            .setPBindings(bindings.data());

        vk::DescriptorSetLayout layout =
            device.createDescriptorSetLayout(dlc);

        return layout;
    }

    std::vector<vk::DescriptorSetLayout> mergeDescriptorSetLayouts(const std::span<const std::span<const result::DescriptorSetLayout>>& layouts,
                                                                   const vk::Device& device) {
        std::map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> set_no_to_bindings;

        for (auto& shader_sets : layouts) {
            for (const result::DescriptorSetLayout& set : shader_sets) {
                if (!set_no_to_bindings.contains(set.set_binding)) {
                    set_no_to_bindings[set.set_binding] = std::vector<vk::DescriptorSetLayoutBinding>();
                }

                for (auto& binding : set.bindings) {
                    set_no_to_bindings[set.set_binding].push_back(binding);
                }
            }
        }

        std::vector<vk::DescriptorSetLayout> out_layouts;

        for (uint32_t i = 0; ; i++) {
            if (!set_no_to_bindings.contains(i)) {
                break;
            }

            auto bindings = set_no_to_bindings[i];

            vk::DescriptorSetLayout layout = createDescriptorSetLayoutFromBindings(bindings, device);

            out_layouts.push_back(layout);
        }

        return out_layouts;
    }
};
