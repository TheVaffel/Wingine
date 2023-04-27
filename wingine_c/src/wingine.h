#ifndef _WG_WINGINE
#define _WG_WINGINE

#include "./types.h"

#include <stdint.h>

EXTERNC

typedef struct wg_wingine_t wg_wingine_t;

wg_wingine_t* wg_create_wingine(uint32_t width, uint32_t height);
void wg_destroy_wingine(wg_wingine_t* wing);

EXTERNC_END

#endif
