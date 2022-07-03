#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

namespace wg::internal::renderPassUtil {

    enum class RenderPassType {
        colorDepth,
        depthOnly
    };

    vk::RenderPass createDefaultRenderPass(RenderPassType type,
                                           const vk::Device& device);


    class RenderPassSetup {
        std::vector<bool> color_clears = { true };
        std::optional<bool> depth_clear = true;

        bool finalize_as_texture = false;

    public:
        uint32_t getNumColorAttachments() const;
        const std::vector<bool>& getColorClears() const;
        bool hasDepthAttachment() const;
        bool getDepthClear() const;
        bool getFinalizeAsTexture() const;

        RenderPassSetup& setNumColorAttachments(uint32_t num_color_attachments);
        RenderPassSetup& setColorClears(const std::vector<bool>& color_clears);
        RenderPassSetup& setEnableDepthAttachment(bool enable);
        RenderPassSetup& setDepthClear(bool enable);
        RenderPassSetup& setFinalizeAsTexture(bool enable);
    };

    vk::RenderPass createRenderPass(const RenderPassSetup& setup, const vk::Device& device);
};
