#pragma once

#include <cstdint>

#include "./types.hpp"

namespace wg::internal {

    struct BasicPipelineSetup {

        BasicPipelineSetup();

        bool depthOnly = false;
        bool enableDepth = true;

        uint32_t width;
        uint32_t height;

        PolygonMode polygonMode = PolygonMode::Fill;

        BasicPipelineSetup& setDepthOnly(bool depthOnly);
        BasicPipelineSetup& setEnableDepth(bool enableDepth);
        BasicPipelineSetup& setWidth(int width);
        BasicPipelineSetup& setHeight(int height);
        BasicPipelineSetup& setPolygonMode(PolygonMode polygonMode);
    };
};
