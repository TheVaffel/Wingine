#pragma once

#include <vulkan/vulkan.hpp>

#include "./Variable.hpp"
#include "./Operation.hpp"

#include "./enums.hpp"
#include "./types/Type.hpp"
#include "./types/PointerType.hpp"

#include <map>

namespace wg::spirv {
    using word = uint32_t;

    namespace result {
        struct DescriptorSetLayout {
            std::vector<vk::DescriptorSetLayoutBinding> bindings;
            word set_binding;
        };
    };

    struct ParseResult {
        ExecutionModel execution_model;

        std::vector<result::DescriptorSetLayout> set_layouts;

        std::map<word, Variable> variables;
        std::map<word, type::PointerType> pointer_types;
        std::map<word, type::Type> base_types;

        // For post processing
        std::vector<Operation> descriptor_set_decorators;
        std::vector<Operation> descriptor_binding_decorators;
    };

    void postProcessParseResult(ParseResult& parse_result);
}
