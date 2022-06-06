#pragma once

#include <cstdint>
#include <array>

namespace wg::internal {
    class CommandRenderPassSettings {
        uint32_t num_color_attachments = 1;
        uint32_t num_depth_attachments = 1;

        bool should_clear_color = true;
        bool should_clear_depth = true;

        float clear_depth;
        std::array<float, 4> clear_color;
    public:

        CommandRenderPassSettings& setNumColorAttachments(uint32_t count);
        CommandRenderPassSettings& setShouldClearColor(bool enable);
        CommandRenderPassSettings& setShouldClearDepth(bool enable);
        CommandRenderPassSettings& setShouldClear(bool enable);
        CommandRenderPassSettings& setDepthOnly();
        CommandRenderPassSettings& setClearColor(const std::array<float, 4>& clear_color);
        CommandRenderPassSettings& setClearDepth(float clear_depth);


        const std::array<float, 4>& getClearColor() const;
        const float& getClearDepth() const;
        bool getShouldClearColor() const;
        bool getShouldClearDepth() const;
        uint32_t getNumColorAttachments() const;
        uint32_t getNumDepthAttachments() const;
    };

    struct CommandControllerSettings {
        CommandRenderPassSettings command_render_pass_settings;
    };
};
