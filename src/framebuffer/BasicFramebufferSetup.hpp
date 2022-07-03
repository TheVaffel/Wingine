#pragma once

namespace wg::internal {
    struct BasicFramebufferSetup {
        bool depth_only = false;
        bool samplable = false;

        BasicFramebufferSetup& setDepthOnly(bool enable);
        BasicFramebufferSetup& setSamplable(bool enable);

        bool getDepthOnly() const;
        bool getSamplable() const;
    };
};
