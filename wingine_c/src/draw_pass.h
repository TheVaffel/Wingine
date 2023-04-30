#ifndef _WG_DRAW_PASS
#define _WG_DRAW_PASS

#include "stdint.h"

#include "./types.h"
#include "./pipeline.h"
#include "./semaphore.h"

EXTERNC

typedef struct wg_command_render_pass_settings_t {
  uint32_t num_color_attachments;
  uint32_t num_depth_attachments;

  uint8_t should_clear_color;
  uint8_t should_clear_depth;
  uint8_t finalize_as_texture;

  float clear_depth;
  float clear_color[4];
} wg_command_render_pass_settings_t;

typedef struct wg_draw_pass_settings_t {
  wg_command_render_pass_settings_t render_pass_settings;
} wg_draw_pass_settings_t;

wg_draw_pass_settings_t wg_default_draw_pass_settings();

typedef struct wg_draw_pass_t wg_draw_pass_t;

wg_draw_pass_t* wg_create_draw_pass(wg_wingine_t* wing, wg_pipeline_t* pipeline, wg_draw_pass_settings_t settings);

void wg_destroy_draw_pass(wg_draw_pass_t* draw_pass);

wg_semaphore_t* wg_draw_pass_create_on_finish_semaphore(wg_draw_pass_t* draw_pass);
void wg_draw_pass_set_wait_semaphores(wg_draw_pass_t* draw_pass, uint32_t num_semaphores, wg_semaphore_t** semaphores);

EXTERNC_END

#endif
