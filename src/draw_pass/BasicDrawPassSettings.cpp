#include "./BasicDrawPassSettings.hpp"

#include <stdexcept>

namespace wg::internal {

    const bool& BasicDrawPassSettings::shouldClearColor() const {
        return this->should_clear_color;
    }

    const bool& BasicDrawPassSettings::shouldClearDepth() const {
        return this->should_clear_depth;
    }


    bool BasicDrawPassSettings::isDepthOnly() const {
        return this->num_color_attachments == 0;
    }

    BasicDrawPassSettings& BasicDrawPassSettings::setDepthOnly(bool enable) {
        if (!enable) {
            throw std::runtime_error("[BasicDrawPassSettings] turning off depthOnly not supported");
        }
        this->num_color_attachments = 0;
        return *this;
    }

    uint32_t BasicDrawPassSettings::getNumColorAttachments() const {
        return this->num_color_attachments;
    }

    const std::array<float, 4>& BasicDrawPassSettings::getClearColor() const {
        return this->clear_color;
    }

    std::array<float, 4>& BasicDrawPassSettings::getClearColor() {
        return this->clear_color;
    }

    const float& BasicDrawPassSettings::getClearDepth() const {
        return this->clear_depth;
    }

    float& BasicDrawPassSettings::getClearDepth() {
        return this->clear_depth;
    }

    BasicDrawPassSettings& BasicDrawPassSettings::setShouldClearColor(bool enable) {
        this->should_clear_color = enable;
        return *this;
    }

    BasicDrawPassSettings& BasicDrawPassSettings::setShouldClearDepth(bool enable) {
        this->should_clear_depth = enable;
        return *this;
    }

    BasicDrawPassSettings& BasicDrawPassSettings::setShouldClear(bool enable) {
        return this->setShouldClearColor(enable).setShouldClearDepth(enable);
    }
};
