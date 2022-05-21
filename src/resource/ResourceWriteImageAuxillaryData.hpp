#pragma once

#include "./IResourceWriteAuxillaryData.hpp"

#include <vulkan/vulkan.hpp>

namespace wg::internal {
    class ResourceWriteImageAuxillaryData : public IResourceWriteAuxillaryData {
    public:
        vk::DescriptorImageInfo image_write_info;
    };
};
