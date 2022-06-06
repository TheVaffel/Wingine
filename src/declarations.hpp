#pragma once

#include <vulkan/vulkan.hpp>

namespace wg {

    // For vertex buffers
    enum ComponentType
    {
        tFloat32,
        tFloat64,
        tInt32,
        tInt64
    };

    struct VertexAttribDesc {
        ComponentType component_type;
        uint32_t binding_num;
        uint32_t num_elements;
        uint32_t stride_in_bytes;
        uint32_t offset_in_bytes;
        bool per_instance = false;
    };

    enum ImageViewType {
        wImageViewColor,
        wImageViewDepth
    };

    class Wingine;
    class RenderFamily;
    class Image;
    class Buffer;
    class SemaphoreChain;
    class ResourceImage;

    template<typename tt>
    class VertexBuffer;

    class IndexBuffer;
};
