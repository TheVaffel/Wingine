#include "./uniform.h"

#include <Wingine.hpp>

#include "./wingine.h"
#include "./types.hpp"
#include "./catch.hpp"

EXTERNC
wg_uniform_t* wg_create_uniform(wg_wingine_t* wing, uint32_t byte_size) {
    return new wg_uniform_t {
        .v = catch_error(wing->wingine.createRawUniformChain(byte_size))
    };
}

void wg_uniform_set_current(wg_uniform_t* uniform, void *data) {
    catch_error(uniform->v->setCurrent(data));
}

void wg_destroy_uniform(wg_uniform_t* uniform) {
    catch_error(delete uniform);
}

EXTERNC_END
