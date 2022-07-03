#include "./CommandControllerSettings.hpp"

namespace wg::internal {

    CommandRenderPassSettings& CommandRenderPassSettings::setNumColorAttachments(uint32_t count) {
        this->num_color_attachments = count;
        return *this;
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setShouldClearColor(bool enable) {
        this->should_clear_color = enable;
        return *this;
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setShouldClearDepth(bool enable) {
        this->should_clear_depth = enable;
        return *this;
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setShouldClear(bool enable) {
        this->should_clear_depth = enable;
        this->should_clear_color = enable;
        return *this;
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setDepthOnly() {
        this->num_depth_attachments = 1;
        return this->setNumColorAttachments(0);
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setClearColor(const std::array<float, 4>& clear_color) {
        this->clear_color = clear_color;
        return *this;
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setClearDepth(float clear_depth) {
        this->clear_depth = clear_depth;
        return *this;
    }

    CommandRenderPassSettings& CommandRenderPassSettings::setFinalizeAsTexture(bool enable) {
        this->finalize_as_texture = enable;
        return *this;
    }

    bool CommandRenderPassSettings::getShouldClearColor() const {
        return this->should_clear_color;
    }

    bool CommandRenderPassSettings::getShouldClearDepth() const {
        return this->should_clear_depth;
    }

    uint32_t CommandRenderPassSettings::getNumColorAttachments() const {
        return this->num_color_attachments;
    }

    uint32_t CommandRenderPassSettings::getNumDepthAttachments() const {
        return this->num_depth_attachments;
    }

    const float& CommandRenderPassSettings::getClearDepth() const {
        return this->clear_depth;
    }

    const std::array<float, 4>& CommandRenderPassSettings::getClearColor() const {
        return this->clear_color;
    }

    bool CommandRenderPassSettings::getFinalizeAsTexture() const {
        return this->finalize_as_texture;
    }
};
