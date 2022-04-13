#include "./BasicDrawPassSettings.hpp"

namespace wg::internal {

    const bool& BasicDrawPassSettings::shouldClearColor() const {
        return this->should_clear_color;
    }

    bool& BasicDrawPassSettings::shouldClearColor() {
        return this->should_clear_color;
    }

    const bool& BasicDrawPassSettings::shouldClearDepth() const {
        return this->should_clear_depth;
    }

    bool& BasicDrawPassSettings::shouldClearDepth() {
        return this->should_clear_depth;
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
