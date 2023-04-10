#include "./Operation.hpp"

#include <span>

#include <flawed_assert.hpp>

#include "./OpCode.hpp"

namespace wg::spirv {

    namespace {
        uint32_t getNumWordsInOperation(uint32_t first_operation_word) {
            return first_operation_word >> 16;
        }

        uint32_t getOpCode(uint32_t first_operation_word) {
            return first_operation_word & OPCODE_MASK;
        }

        std::vector<std::vector<uint32_t>> splitToOperations(const std::span<const uint32_t> stream) {
            std::vector<std::vector<uint32_t>> operations;

            // Skip preamble
            uint32_t current_word = 5;

            while (current_word < stream.size()) {
                uint32_t num_words_in_operation = getNumWordsInOperation(stream[current_word]);

                std::vector<uint32_t> current_operation;

                for (uint32_t i = current_word; i < current_word + num_words_in_operation; i++) {
                    current_operation.push_back(stream[i]);
                }

                operations.push_back(current_operation);
                current_word += num_words_in_operation;
            }

            return operations;
        }
    };

    Operation::Operation(const std::span<const uint32_t> operation_bytecode) {
        fl_assert_gt(operation_bytecode.size(), 0u);

        this->total_word_count = getNumWordsInOperation(operation_bytecode[0]);
        this->opcode = (OpCode) wg::spirv::getOpCode(operation_bytecode[0]);

        for (uint32_t i = 1; i < operation_bytecode.size(); i++) {
            this->arguments.push_back(operation_bytecode[i]);
        }
    }

    OpCode Operation::getOpCode() const {
        return this->opcode;
    }

    uint32_t Operation::getWordCount() const {
        return this->total_word_count;
    }

    const std::span<const uint32_t> Operation::getArguments() const {
        return this->arguments;
    }


    uint32_t Operation::getArg(uint32_t ind) const {
        return this->arguments[ind];
    }


    namespace operation {
        std::vector<Operation> parseOperations(const std::span<const uint32_t> bytecode) {
            auto operation_byte_codes = splitToOperations(bytecode);

            std::vector<Operation> operations;
            for (auto operation_byte_code : operation_byte_codes) {
                operations.emplace_back(operation_byte_code);
            }

            return operations;
        }
    }
};
