#include "./types.hpp"

#include <stdint.h>

#include "./common.hpp"

extern "C" {
    win_winval_t* win_create_winval(uint32_t width, uint32_t height) {
        return new win_winval_t {
            .winval = Winval(width, height)
        };
    }

    uint8_t win_winval_is_window_open(win_winval_t* win) {
        return exitOnFail(( [&win] { return (int)win->winval.isOpen(); }));
    }

    uint8_t win_winval_is_key_pressed(win_winval_t* win, uint32_t key) {
        return exitOnFail(( [&win, &key] { return (int)win->winval.isKeyPressed(key); }));
    }

    void win_winval_sleep_milliseconds(win_winval_t* win, uint32_t millis) {
        return exitOnFail(( [&win, &millis] { return win->winval.sleepMilliseconds(millis); }));
    }

    void win_winval_flush_events(win_winval_t* win) {
        return exitOnFail(( [&win] { return win->winval.flushEvents(); }));
    }

    void win_destroy_winval(win_winval_t* winval) {
        delete winval;
    }
}
