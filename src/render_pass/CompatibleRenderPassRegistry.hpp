#pragma once

#include <vulkan/vulkan.hpp>

#include <map>

#include "../core/DeviceManager.hpp"

#include "./renderPassUtil.hpp"

namespace wg::internal {


    /*
     * Keep some render passes handy so that we don't need to
     * recreate them at a later point
     */
    class CompatibleRenderPassRegistry {
        std::map<renderPassUtil::RenderPassType, vk::RenderPass> compatibleRenderPassMap;
        std::shared_ptr<const DeviceManager> device_manager;

    public:

        CompatibleRenderPassRegistry(std::shared_ptr<const DeviceManager> device_manager);
        ~CompatibleRenderPassRegistry();

        vk::RenderPass ensureAndGetRenderPass(renderPassUtil::RenderPassType type);

        bool hasRenderPassType(renderPassUtil::RenderPassType type) const;
        const vk::RenderPass getRenderPass(renderPassUtil::RenderPassType type) const;
    };
};
