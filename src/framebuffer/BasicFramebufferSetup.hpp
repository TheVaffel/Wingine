#pragma once

namespace wg::internal {
    struct BasicFramebufferSetup {
        bool depth_only = false;

        BasicFramebufferSetup& setDepthOnly(bool enable);

        bool getDepthOnly() const;
    };
};
