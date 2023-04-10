#pragma once

#include <span>
#include <cstdint>
#include <map>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "./Operation.hpp"
#include "./ParseResult.hpp"

namespace wg::spirv {

    using DescriptorSetLayout = result::DescriptorSetLayout;

    class SpirvModule {

        ParseResult parse_result;

        std::map<OpCode, std::function<void(Operation operation, ParseResult& parse_result)>> parse_functions;

        void parseOpCode(Operation operation);

        void initializeOpCodeParsers();
        void parseByteCode(const std::span<const uint32_t> spirv_bytecode);

    public:

        SpirvModule(const std::span<const uint32_t> spirv_bytecode);

        const ExecutionModel getExecutionModel() const;
        const std::vector<Variable> getVariables() const;

        const std::vector<DescriptorSetLayout>& getDescriptorSetLayouts() const;
    };

};
