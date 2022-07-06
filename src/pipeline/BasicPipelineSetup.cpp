#include "./BasicPipelineSetup.hpp"

namespace wg::internal {

    BasicPipelineSetup::BasicPipelineSetup()
        : width(0), height(0) { }

    BasicPipelineSetup& BasicPipelineSetup::setDepthOnly(bool depthOnly) {
        this->depthOnly = depthOnly;
        return *this;
    }

    BasicPipelineSetup& BasicPipelineSetup::setEnableDepth(bool enableDepth) {
        this->enableDepth = enableDepth;
        return *this;
    }

    BasicPipelineSetup& BasicPipelineSetup::setWidth(int width) {
        this->width = width;
        return *this;
    }

    BasicPipelineSetup& BasicPipelineSetup::setHeight(int height) {
        this->height = height;
        return *this;
    }

    BasicPipelineSetup& BasicPipelineSetup::setPolygonMode(PolygonMode polygonMode) {
        this->polygonMode = polygonMode;
        return *this;
    }
};
