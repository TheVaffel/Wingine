#include "./semaphore.h"

#include "./types.hpp"
#include "./wingine.h"
#include "./catch.hpp"

extern "C" {

    void wg_destroy_semaphore(wg_semaphore_t* semaphore) {
        catch_error(delete semaphore);
    }
};
