#pragma once

#include <array>
#include <cstdint>

namespace wg::internal {
    class BasicDrawPassSettings {
        bool should_clear_color = true;
        bool should_clear_depth = true;

        std::array<float, 4> clear_color = { 0.2f, 0.2f, 0.2f, 1.0f };
        float clear_depth = 1.0f;

        uint32_t num_color_attachments = 1;

    public:
        const bool& shouldClearColor() const;

        const bool& shouldClearDepth() const;

        bool isDepthOnly() const;
        BasicDrawPassSettings& setDepthOnly(bool enable);
        uint32_t getNumColorAttachments() const;

        std::array<float, 4>& getClearColor();
        const std::array<float, 4>& getClearColor() const;

        float& getClearDepth();
        const float& getClearDepth() const;

        BasicDrawPassSettings& setShouldClearColor(bool enable);
        BasicDrawPassSettings& setShouldClearDepth(bool enable);
        BasicDrawPassSettings& setShouldClear(bool enable);
        BasicDrawPassSettings& setClearColor(const std::array<float, 4>& clear_color);
        BasicDrawPassSettings& setClearDepth(float clear_depth);
    };
};
