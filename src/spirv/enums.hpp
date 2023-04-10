#pragma once

#include <vulkan/vulkan.hpp>

// ???
#undef Bool

namespace wg::spirv {

    enum class BaseType {
        Void = 19,
        Bool = 20,
        Int = 21,
        Float = 22,
        Vector = 23,
        Matrix = 24,
        Image = 25,
        Sampler = 26,
        SampledImage = 27,
        Array = 28,
        RuntimeArray = 29,
        Struct = 30,
        Opaque = 31
    };

    enum class Decoration {
        Binding = 33,
        DescriptorSet = 34
    };

    enum class ExecutionModel {
        Vertex = 0,
        Fragment = 4,
        GLCompute = 5,
    };

    enum class StorageClass {
        UniformConstant = 0,
        Input = 1,
        Uniform = 2,
        Output = 3,
        StorageBuffer = 12
    };
};
