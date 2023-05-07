#ifndef _WG_COMMAND
#define _WG_COMMAND

#include "./types.h"

#include "./buffer.h"
#include "./draw_pass.h"
#include "./framebuffer.h"

EXTERNC

typedef struct wg_command_t wg_command_t;

typedef struct wg_resource_binding_t {
  uint32_t binding;
  void* resource_binding;
} wg_resource_binding_t;

wg_command_t* wg_draw_pass_get_command(wg_draw_pass_t *draw_pass);

void wg_cmd_start_recording(wg_command_t* command, wg_framebuffer_t* framebuffer);
void wg_cmd_bind_resource_set(wg_command_t* command, uint32_t binding_index, uint32_t num_bindings, wg_resource_binding_t *bindings);
void wg_cmd_draw(wg_command_t* command, uint32_t num_buffers, wg_vertex_buffer_t **vertex_buffers, wg_index_buffer_t *index_buffer);
void wg_cmd_end_recording(wg_command_t *command);

EXTERNC_END

#endif
