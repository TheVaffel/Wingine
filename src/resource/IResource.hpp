#pragma once

#include <vulkan/vulkan.hpp>

#include "./IResourceWriteAuxillaryData.hpp"

namespace wg::internal {
    class IResource {
    public:
        [[nodiscard]]
        virtual std::unique_ptr<IResourceWriteAuxillaryData>
        writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const = 0;

        virtual ~IResource() = default;
    };
};
