#ifndef _WG_WINGINE
#define _WG_WINGINE

#include "./types.h"
#include "./semaphore.h"
#include "./winval.h"

EXTERNC

typedef struct wg_wingine_t wg_wingine_t;

wg_wingine_t* wg_create_wingine_headless(uint32_t width, uint32_t height, const char* app_name);
wg_wingine_t* wg_create_wingine_with_winval(win_winval_t* winval, const char* app_name);
void wg_destroy_wingine(wg_wingine_t* wing);

void wg_wingine_wait_idle(wg_wingine_t* wing);

wg_semaphore_t* wg_wingine_create_image_ready_semaphore(wg_wingine_t* wing);
void wg_wingine_set_present_wait_semaphores(wg_wingine_t* wing, uint32_t num_semaphores, wg_semaphore_t** raw_semaphores);

void wg_wingine_present(wg_wingine_t* wing);
void wg_wingine_copy_last_rendered_image(wg_wingine_t* wing, uint32_t* data);

uint32_t wg_wingine_get_window_width(wg_wingine_t* wing);
uint32_t wg_wingine_get_window_height(wg_wingine_t* wing);

EXTERNC_END

#endif
