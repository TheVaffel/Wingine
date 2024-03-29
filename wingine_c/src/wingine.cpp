#include "./wingine.h"
#include <Wingine.hpp>

#include "./types.hpp"

extern "C" {

    wg_wingine_t* wg_create_wingine(uint32_t width, uint32_t height) {
        std::unique_ptr<Winval> win = std::make_unique<Winval>(width, height);
        return new wg_wingine_t {
            .wingine = wg::Wingine(*win),
            .winval = std::move(win)
        };
    }

    void wg_destroy_wingine(wg_wingine_t* wing) {
        delete wing;
    }

    wg_semaphore_t* wg_wingine_create_image_ready_semaphore(wg_wingine_t* wing) {
        return new wg_semaphore_t {
            .v = wing->wingine.createAndAddImageReadySemaphore()
        };
    }

    void wg_wingine_wait_idle(wg_wingine_t* wing) {
        wing->wingine.waitIdle();
    }

    void wg_wingine_set_present_wait_semaphores(wg_wingine_t* wing, uint32_t num_semaphores, wg_semaphore_t** raw_semaphores) {
        std::vector<wg::Semaphore> semaphores(num_semaphores);
        for (uint32_t i = 0; i < num_semaphores; i++) {
            semaphores[i] = raw_semaphores[i]->v;
        }
        wing->wingine.setPresentWaitForSemaphores(semaphores);
    }

    void wg_wingine_present(wg_wingine_t* wing) {
        return wing->wingine.present();
    }

    uint8_t wg_wingine_is_window_open(wg_wingine_t* wing) {
        return (int)wing->winval->isOpen();
    }

    uint8_t wg_wingine_is_key_pressed(wg_wingine_t* wing, uint32_t key) {
        return (int)wing->winval->isKeyPressed(key);
    }

    void wg_wingine_sleep_milliseconds(wg_wingine_t* wing, uint32_t millis) {
        return wing->winval->sleepMilliseconds(millis);
    }

    void wg_wingine_flush_events(wg_wingine_t* wing) {
        return wing->winval->flushEvents();
    }
};
