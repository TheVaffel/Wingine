#include "./framebuffer.h"

#include "./types.hpp"
#include <Wingine.hpp>

extern "C" {
    wg_framebuffer_t* wg_get_default_framebuffer(wg_wingine_t* wing) {
        return (wg_framebuffer_t*)&wing->wingine.getDefaultFramebufferChain();
    }
};
