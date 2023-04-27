#include "./uniform.h"

#include <Wingine.hpp>

#include "./wingine.h"
#include "./types.hpp"

EXTERNC
wg_uniform_t* wg_create_uniform(wg_wingine_t* wing, uint32_t byte_size) {
    return new wg_uniform_t {
        .v = wing->wingine.createRawUniformChain(byte_size)
    };
}

void wg_set_current_uniform(wg_uniform_t* uniform, void *data) {
    uniform->v->setCurrent(data);
}

void wg_destroy_uniform(wg_uniform_t* uniform) {
    delete uniform;
}

EXTERNC_END
