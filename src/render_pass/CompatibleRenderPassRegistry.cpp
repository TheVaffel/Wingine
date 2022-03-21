#include "./CompatibleRenderPassRegistry.hpp"

namespace wg::internal {

    CompatibleRenderPassRegistry::CompatibleRenderPassRegistry(std::shared_ptr<const DeviceManager> device_manager) : device_manager(device_manager) {}

    CompatibleRenderPassRegistry::~CompatibleRenderPassRegistry() {
        for (auto it : this->compatibleRenderPassMap) {
            this->device_manager->getDevice().destroy(it.second);
        }
    }

    bool CompatibleRenderPassRegistry::hasRenderPassType(renderPassUtil::RenderPassType type) const {
        return this->compatibleRenderPassMap.find(type) != this->compatibleRenderPassMap.end();
    }

    vk::RenderPass CompatibleRenderPassRegistry::ensureAndGetRenderPass(renderPassUtil::RenderPassType type) {
        if (!hasRenderPassType(type)) {
            this->compatibleRenderPassMap[type] =
                renderPassUtil::createDefaultRenderPass(type, this->device_manager->getDevice());
        }

        return this->compatibleRenderPassMap[type];
    }

    const vk::RenderPass CompatibleRenderPassRegistry::getRenderPass(renderPassUtil::RenderPassType type) const {
        if (!this->hasRenderPassType(type)) {
            throw std::runtime_error("[CompatibleRenderPassRegistry] Render pass not found for this type");
        }

        return this->compatibleRenderPassMap.at(type);
    }
};
