#pragma once

#include <array>
#include <cstdint>

#include "../command/CommandControllerSettings.hpp"

namespace wg::internal {
    class BasicDrawPassSettings {
    public:
        CommandRenderPassSettings render_pass_settings;
    };
};
