#include "./shader.h"

#include <Wingine.hpp>

#include "./wingine.h"
#include "./types.hpp"

EXTERNC

wg_shader_t* wg_create_shader(wg_wingine_t* wing, wg_shader_stage stage, uint32_t* spv, uint32_t num_words) {
    std::vector<uint32_t> spirv(spv, spv + num_words);

    return new wg_shader_t {
        .v = wing->wingine.createShader((wg::ShaderStage)stage, spirv)
    };
}

void wg_destroy_shader(wg_shader_t* shader) {
    delete shader;
}

EXTERNC_END
