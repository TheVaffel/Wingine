#include "./types.h"

#include <vulkan/vulkan.h>

EXTERNC

typedef enum wg_shader_stage {
  WG_SHADER_STAGE_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
  WG_SHADER_STAGE_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
  WG_SHADER_STAGE_COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT
} wg_shader_stage;

typedef struct wg_shader_t wg_shader_t;

wg_shader_t* wg_create_shader(wg_wingine_t* wing, wg_shader_stage stage, uint32_t* spv, uint32_t num_words);
void wg_destroy_shader(wg_shader_t* stage);

EXTERNC_END
