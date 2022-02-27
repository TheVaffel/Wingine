#pragma once

#include <vulkan/vulkan.hpp>

#include <map>

#include "./DeviceManager.hpp"

namespace wg::internal {
    enum class RenderPassType {
        renColorDepth,
        renDepth
    };


    /*
     * Keep some render passes handy so that we don't need to
     * recreate them at a later point
     */
    class CompatibleRenderPassRegistry {
        std::map<RenderPassType, vk::RenderPass> compatibleRenderPassMap;
        std::shared_ptr<const DeviceManager> device_manager;

    public:

        CompatibleRenderPassRegistry(std::shared_ptr<const DeviceManager> device_manager);
        ~CompatibleRenderPassRegistry();

        bool hasRenderPassType(RenderPassType type) const;
        void registerRenderPassType(RenderPassType type, vk::RenderPass);
        const vk::RenderPass getRenderPass(RenderPassType type) const;
    };
};
