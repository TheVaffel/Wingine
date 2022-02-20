#include "./CompatibleRenderPassRegistry.hpp"

namespace wg {

    CompatibleRenderPassRegistry::CompatibleRenderPassRegistry(vk::Device device) : device(device) {}

    CompatibleRenderPassRegistry::~CompatibleRenderPassRegistry() {
        for (auto it : this->compatibleRenderPassMap) {
            this->device.destroy(it.second, nullptr);
        }
    }

    bool CompatibleRenderPassRegistry::hasRenderPassType(RenderPassType type) const {
        return this->compatibleRenderPassMap.find(type) != this->compatibleRenderPassMap.end();
    }

    void CompatibleRenderPassRegistry::registerRenderPassType(RenderPassType type, vk::RenderPass render_pass) {
        if (hasRenderPassType(type)) {
            throw std::runtime_error("[CompatibleRenderPassRegistry] Render pass already exists in registry!");
        }
        this->compatibleRenderPassMap[type] = render_pass;
    }

    const vk::RenderPass CompatibleRenderPassRegistry::getRenderPass(RenderPassType type) const {
        if (!this->hasRenderPassType(type)) {
            throw std::runtime_error("[CompatibleRenderPassRegistry] Render pass not found for this type");
        }

        return this->compatibleRenderPassMap.at(type);
    }
};
