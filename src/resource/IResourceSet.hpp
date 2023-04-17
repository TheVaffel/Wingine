#pragma once

#include <vulkan/vulkan.hpp>

#include "./IResource.hpp"

namespace wg::internal {
    class IResourceSet {
    public:
        virtual vk::DescriptorSet getDescriptorSet() const = 0;

        virtual void set(const std::vector<std::shared_ptr<IResource>>& resources) = 0;
    };
};
