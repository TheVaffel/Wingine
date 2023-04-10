#pragma once

#include <vector>
#include <cstdint>
#include <span>

#include "./OpCode.hpp"

namespace wg::spirv {
    class Operation {

        std::vector<uint32_t> arguments;

        uint32_t total_word_count;
        OpCode opcode;

    public:

        Operation(const std::span<const uint32_t> operation_bytecode);

        OpCode getOpCode() const;
        uint32_t getWordCount() const;

        const std::span<const uint32_t> getArguments() const;
        uint32_t getArg(uint32_t ind) const;
    };

    namespace operation {
        std::vector<Operation> parseOperations(const std::span<const uint32_t> bytecode);
    };
};
