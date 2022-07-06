#pragma once

#include <vulkan/vulkan.hpp>

namespace wg {

    enum ImageViewType {
        wImageViewColor,
        wImageViewDepth
    };

    class Wingine;
    class Image;
    class Buffer;
    class SemaphoreChain;
    class ResourceImage;

    template<typename tt>
    class VertexBuffer;

    class IndexBuffer;
};
