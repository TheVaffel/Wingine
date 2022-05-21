#pragma once

#include "./IResourceWriteAuxillaryData.hpp"

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class ResourceWriteBufferAuxillaryData : public IResourceWriteAuxillaryData {
    public:
        vk::DescriptorBufferInfo buffer_info;
    };
};
