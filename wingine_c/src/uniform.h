#ifndef WG_UNIFORM
#define WG_UNIFORM

#include "./types.h"

EXTERNC

typedef struct wg_uniform_t wg_uniform_t;

wg_uniform_t* wg_create_uniform(wg_wingine_t* wing, uint32_t byte_size);
void wg_uniform_set_current(wg_uniform_t* chain, void *data);
void wg_destroy_uniform(wg_uniform_t* uniform);

EXTERNC_END

#endif // WG_UNIFORM
