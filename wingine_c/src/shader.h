#ifndef _WG_SHADER
#define _WG_SHADER

#include "./types.h"
#include "./wingine.h"

EXTERNC

typedef enum wg_shader_stage {
  WG_SHADER_STAGE_VERTEX = 0,
  WG_SHADER_STAGE_FRAGMENT = 1,
  WG_SHADER_STAGE_COMPUTE = 2
} wg_shader_stage;

typedef struct wg_shader_t wg_shader_t;

wg_shader_t* wg_create_shader(wg_wingine_t* wing, wg_shader_stage stage, const uint32_t* spv, uint32_t num_words);
void wg_destroy_shader(wg_shader_t* stage);

EXTERNC_END

#endif
