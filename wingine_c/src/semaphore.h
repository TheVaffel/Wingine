#ifndef _WG_SEMAPHORE
#define _WG_SEMAPHORE

#include "./types.h"
#include "./wingine.h"

EXTERNC

typedef struct wg_semaphore_t wg_semaphore_t;

void wg_destroy_semaphore(wg_semaphore_t* semaphore);

EXTERNC_END

#endif
