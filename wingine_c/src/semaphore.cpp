#include "./semaphore.h"

#include "./types.hpp"
#include "./wingine.h"
extern "C" {

    void wg_destroy_semaphore(wg_semaphore_t* semaphore) {
        delete semaphore;
    }
};
