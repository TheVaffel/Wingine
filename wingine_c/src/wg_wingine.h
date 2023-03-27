#ifndef _WG_WINGINE
#define _WG_WINGINE

#include "./types.h"

#include <stdint.h>

EXTERNC

struct wg_wingine_t {
  void* winval;
  void* wingine;
};

int wg_initialize_wingine(wg_wingine_t* wing, uint32_t width, uint32_t height);
int wg_destroy_wingine(wg_wingine_t* wing);

EXTERNC_END

#endif
