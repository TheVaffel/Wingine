#pragma once

#include <cstdint>

namespace wg::spirv {
    const uint32_t OPCODE_MASK = (1 << 16) - 1;

    enum class OpCode : uint32_t {
        EntryPoint = 15,
        ExecutionMode = 16,
        TypeVoid = 19,
        TypeBool = 20,
        TypeInt = 21,
        TypeFloat = 22,
        TypeVector = 23,
        TypeMatrix = 24,
        TypeImage = 25,
        TypeSampler = 26,
        TypeSampledImage = 27,
        TypeArray = 28,
        TypeRuntimeArray = 29,
        TypeStruct = 30,
        TypeOpaque = 31,
        TypePointer = 32,
        Variable = 59,
        Decorate = 71
    };
};
