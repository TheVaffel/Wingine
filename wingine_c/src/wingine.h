#ifndef _WG_WINGINE
#define _WG_WINGINE

#include "./types.h"
#include "./semaphore.h"

#include <stdint.h>

EXTERNC

typedef struct wg_wingine_t wg_wingine_t;

wg_wingine_t* wg_create_wingine(uint32_t width, uint32_t height);
void wg_destroy_wingine(wg_wingine_t* wing);

void wg_wingine_wait_idle(wg_wingine_t* wing);

wg_semaphore_t* wg_wingine_create_image_ready_semaphore(wg_wingine_t* wing);
void wg_wingine_set_present_wait_semaphores(wg_wingine_t* wing, uint32_t num_semaphores, wg_semaphore_t** raw_semaphores);

void wg_wingine_present(wg_wingine_t* wing);

uint8_t wg_wingine_is_window_open(wg_wingine_t* wing);
uint8_t wg_wingine_is_key_pressed(wg_wingine_t* wing, uint32_t key);
void wg_wingine_sleep_milliseconds(wg_wingine_t* wing, uint32_t millis);
void wg_wingine_flush_events(wg_wingine_t* wing);

EXTERNC_END

#endif
