#pragma once

#include <vulkan/vulkan.hpp>

#include <map>

namespace wg {
    enum RenderPassType {
        renColorDepth,
        renDepth
    };


    /*
     * Keep some render passes handy so that we don't need to
     * recreate them at a later point
     */
    class CompatibleRenderPassRegistry {
        std::map<RenderPassType, vk::RenderPass> compatibleRenderPassMap;
        vk::Device device;

    public:

        CompatibleRenderPassRegistry(vk::Device device);
        ~CompatibleRenderPassRegistry();

        bool hasRenderPassType(RenderPassType type) const;
        void registerRenderPassType(RenderPassType type, vk::RenderPass);
        const vk::RenderPass getRenderPass(RenderPassType type) const;
    };
};
