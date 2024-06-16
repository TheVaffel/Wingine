#include "./wingine.h"
#include <Wingine.hpp>

#include "./types.hpp"

#include "./common.hpp"

extern "C" {
    wg_wingine_t* wg_create_wingine_headless(uint32_t width, uint32_t height, const char* app_name) {
        return exitOnFail(([&width, &height, &app_name] {
            return new wg_wingine_t {
                .wingine = wg::Wingine(width, height, app_name)
            };
        }));
    }

    wg_wingine_t* wg_create_wingine_with_winval(win_winval_t* winval, const char* app_name) {
        return exitOnFail(( [&winval] {
            return new wg_wingine_t {
                .wingine = wg::Wingine(winval->winval)
            };
        }));
    }

    void wg_destroy_wingine(wg_wingine_t* wing) {
        delete wing;
    }

    wg_semaphore_t* wg_wingine_create_image_ready_semaphore(wg_wingine_t* wing) {
        return new wg_semaphore_t {
            .v = exitOnFail(( [&wing] { return wing->wingine.createAndAddImageReadySemaphore(); }))
        };
    }

    void wg_wingine_wait_idle(wg_wingine_t* wing) {
        return exitOnFail(( [&wing] { wing->wingine.waitIdle(); }));
    }

    void wg_wingine_set_present_wait_semaphores(wg_wingine_t* wing, uint32_t num_semaphores, wg_semaphore_t** raw_semaphores) {
        std::vector<wg::Semaphore> semaphores(num_semaphores);
        for (uint32_t i = 0; i < num_semaphores; i++) {
            semaphores[i] = raw_semaphores[i]->v;
        }
        return exitOnFail(( [&wing, &semaphores] { wing->wingine.setPresentWaitForSemaphores(semaphores); }));
    }

    void wg_wingine_present(wg_wingine_t* wing) {
        return exitOnFail(( [&wing] { return wing->wingine.present(); }));
    }

    uint32_t wg_wingine_get_window_width(wg_wingine_t* wing) {
        return exitOnFail(( [&wing] { return wing->wingine.getWindowWidth(); }));
    }

    uint32_t wg_wingine_get_window_height(wg_wingine_t* wing) {
        return exitOnFail(([&wing] { return wing->wingine.getWindowHeight(); }));
    }

    void wg_wingine_copy_last_rendered_image(wg_wingine_t* wing, uint32_t *data) {
        return exitOnFail(([&wing, &data] { return wing->wingine.copyLastRenderedImage(data); }));
    }
};
