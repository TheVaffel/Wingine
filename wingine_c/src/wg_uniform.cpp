#include "./wg_uniform.h"
#include <Wingine.hpp>

extern "C" {
    int wg_create_uniform(wg_wingine_t* wing, wg_uniform_chain_t* chain, uint32_t byte_size) {
        using TT = char[byte_size];
        auto sp = new wg::
    }

    int wg_set_current_uniform(wg_uniform_chain_t* chain, void *data) {

    }

    int wg_destroy_uniform(wg_uniform_chain_t* chain) {

    }

};
