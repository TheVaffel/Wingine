#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class IDimensionable {
    public:
        virtual vk::Extent2D getDimensions() const = 0;
    };
};
