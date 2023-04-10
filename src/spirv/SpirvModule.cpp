#include "./SpirvModule.hpp"

#include "./Operation.hpp"
#include "./OpCode.hpp"
#include "./header.hpp"
#include "./variable_parser.hpp"
#include "./decorator_parser.hpp"
#include "./types/parse_type.hpp"

#include <iostream>

namespace wg::spirv {

    void SpirvModule::initializeOpCodeParsers() {

        using ft = std::function<void(Operation, ParseResult&)>;

        this->parse_functions[OpCode::EntryPoint] = ft(header::parseEntryPoint);
        this->parse_functions[OpCode::Variable] = ft(variable::parseVariable);
        this->parse_functions[OpCode::Decorate] = ft(decorator::parseDecorator);
        this->parse_functions[OpCode::TypeVoid] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeBool] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeInt] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeFloat] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeVector] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeMatrix] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeImage] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeSampler] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeSampledImage] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeArray] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeRuntimeArray] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeStruct] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypeOpaque] = ft(type::parseBaseType);
        this->parse_functions[OpCode::TypePointer] = ft(type::parsePointerType);
    }

    void SpirvModule::parseByteCode(const std::span<const uint32_t> spirv_bytecode) {
        std::vector<Operation> operations = operation::parseOperations(spirv_bytecode);

        for (const Operation& operation : operations) {
            if (this->parse_functions.find(operation.getOpCode()) != this->parse_functions.end()) {
                this->parse_functions[operation.getOpCode()](operation, this->parse_result);
            }
        }

        postProcessParseResult(this->parse_result);
    }

    SpirvModule::SpirvModule(const std::span<const uint32_t> byte_code) {
        this->initializeOpCodeParsers();
        this->parseByteCode(byte_code);
    }

    const ExecutionModel SpirvModule::getExecutionModel() const {
        return this->parse_result.execution_model;
    }

    const std::vector<Variable> SpirvModule::getVariables() const {
        std::vector<Variable> vars;
        for (auto it = this->parse_result.variables.begin(); it != this->parse_result.variables.end(); ++it) {
            vars.push_back((*it).second);
        }

        return vars;
    }

    const std::vector<result::DescriptorSetLayout>& SpirvModule::getDescriptorSetLayouts() const {
        return this->parse_result.set_layouts;
    }
}
