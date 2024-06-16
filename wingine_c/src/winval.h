
#ifndef _WIN_WINVAL
#define _WIN_WINVAL

#include "./types.h"

EXTERNC

typedef struct win_winval_t win_winval_t;

win_winval_t* win_create_winval(uint32_t width, uint32_t height);

uint8_t win_winval_is_window_open(win_winval_t* win);
uint8_t win_winval_is_key_pressed(win_winval_t* win, uint32_t key);
void win_winval_sleep_milliseconds(win_winval_t* win, uint32_t millis);
void win_winval_flush_events(win_winval_t* win);

void win_destroy_winval(win_winval_t* winval);

EXTERNC_END

#endif
