#ifndef _WG_FRAMEBUFFER
#define _WG_FRAMEBUFFER

#include "./types.h"
#include "./wingine.h"

EXTERNC

typedef struct wg_framebuffer_t wg_framebuffer_t;

wg_framebuffer_t* wg_get_default_framebuffer(wg_wingine_t* wing);

EXTERNC_END
#endif
