#include "./ParseResult.hpp"

#include "./util.hpp"
#include "./Variable.hpp"

#include <map>
#include <set>
#include <algorithm>

#include <iostream>

namespace wg::spirv {
    namespace {
        std::vector<uint32_t> deduplicate(const std::vector<uint32_t>& v) {
            std::set<uint32_t> s;
            std::vector<uint32_t> res;
            for (uint32_t i : v) {
                s.insert(i);
            }

            for (uint32_t i : s) {
                res.push_back(i);
            }

            return res;
        }

        void parseDecorators(ParseResult& parse_result) {
            std::map<uint32_t, uint32_t> variable_to_descriptor_set;
            std::map<uint32_t, uint32_t> variable_to_descriptor_binding;

            for ( auto& decorator : parse_result.descriptor_set_decorators) {
                uint32_t target = decorator.getArg(0);

                variable_to_descriptor_set[target] = decorator.getArguments()[2];
            }

            for (auto& decorator : parse_result.descriptor_binding_decorators) {
                uint32_t target = decorator.getArg(0);

                variable_to_descriptor_binding[target] = decorator.getArguments()[2];
            }

            std::vector<uint32_t> descriptor_set_ids;
            for (auto &pp : variable_to_descriptor_set) {
                descriptor_set_ids.push_back(pp.second);
            }

            descriptor_set_ids = deduplicate(descriptor_set_ids);

            std::map<uint32_t, result::DescriptorSetLayout> id_to_descriptor_set;
            for (uint32_t id : descriptor_set_ids) {
                result::DescriptorSetLayout desc;
                desc.set_binding = id;
                id_to_descriptor_set[id] = desc;
            }

            for (auto& variable : parse_result.variables) {
                word identifier = variable.first;

                if (variable_to_descriptor_set.contains(identifier) &&
                    !variable_to_descriptor_binding.contains(identifier)) {
                    // Assume variable without binding number is binding 0

                    auto& desc_layout = id_to_descriptor_set[variable_to_descriptor_set[identifier]];

                    vk::DescriptorSetLayoutBinding binding;
                    binding.setBinding(0)
                        .setDescriptorCount(1)
                        .setStageFlags(vk::ShaderStageFlagBits::eAll)
                        .setDescriptorType(util::getDescriptorType(variable.second, parse_result));

                    desc_layout.bindings.push_back(binding);
                } else if (variable_to_descriptor_set.contains(identifier) &&
                    variable_to_descriptor_binding.contains(identifier)) {
                    auto& desc_layout = id_to_descriptor_set[variable_to_descriptor_set[identifier]];

                    vk::DescriptorSetLayoutBinding binding;
                    binding.setBinding(variable_to_descriptor_binding[identifier])
                        .setDescriptorCount(1)
                        .setStageFlags(vk::ShaderStageFlagBits::eAll)
                        .setDescriptorType(util::getDescriptorType(variable.second, parse_result));

                    desc_layout.bindings.push_back(binding);
                }
            }

            for (auto &pp : id_to_descriptor_set) {
                parse_result.set_layouts.push_back(pp.second);
            }
        }
    };

    void postProcessParseResult(ParseResult& parse_result) {
        parseDecorators(parse_result);
    }
};
