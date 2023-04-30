
#include "../wingine_c.h"

#include <stdint.h>
#include <stdio.h>
#include <WinvalKeys.h>

int main() {

  const int width = 800, height = 800;

  wg_wingine_t* wing = wg_create_wingine(width, height);

  const int num_points = 3;
  const int num_triangles = 1;

  float positions[3 * 4] = {
    1.0f, -1.0f, -2.5f, 1.0f,
    -1.0f, -1.0f, -2.5f, 1.0f,
    0.0f, 1.0f, -2.5f, 1.0f,
  };

  float colors[3 * 4] = {
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
  };

  uint32_t indices[1 * 3] = {
    0, 1, 2,
  };

  wg_vertex_buffer_t *position_buffer = wg_create_vertex_buffer(wing, num_points * sizeof(float) * 4);
  wg_set_vertex_buffer(position_buffer, positions, 0, num_points * sizeof(float) * 4);

  wg_vertex_buffer_t *color_buffer = wg_create_vertex_buffer(wing, num_points * sizeof(float) * 4);
  wg_set_vertex_buffer(color_buffer, colors, 0, num_points * sizeof(float) * 4);

  wg_index_buffer_t *index_buffer = wg_create_index_buffer(wing, num_triangles * 3);
  wg_set_index_buffer(index_buffer, indices, 0, num_triangles * 3);

  wg_uniform_t *camera_uniform = wg_create_uniform(wing, 16 * sizeof(float));

  wg_vertex_attrib_desc_t attrib_desc[2] = {
    wg_create_attrib_desc(0, WG_COMPONENT_TYPE_FLOAT32, 4, 4 * sizeof(float), 0),
    wg_create_attrib_desc(1, WG_COMPONENT_TYPE_FLOAT32, 4, 4 * sizeof(float), 0)
  };

  uint32_t vertex_words, fragment_words;
  uint32_t* vertex_spv = wg_read_spv("./vertex.spv", &vertex_words);
  uint32_t* fragment_spv = wg_read_spv("./frag.spv", &fragment_words);

  wg_shader_t *vertex_shader = wg_create_shader(wing, WG_SHADER_STAGE_VERTEX, vertex_spv, vertex_words);

  wg_shader_t *fragment_shader = wg_create_shader(wing, WG_SHADER_STAGE_FRAGMENT, fragment_spv, fragment_words);

  wg_shader_t *shaders[2] = { vertex_shader, fragment_shader };

  wg_pipeline_t *pipeline = wg_create_pipeline(wing, 2, attrib_desc, 2, shaders);

  wg_draw_pass_settings_t draw_pass_settings = wg_default_draw_pass_settings();
  draw_pass_settings.render_pass_settings.should_clear_color = 1;
  draw_pass_settings.render_pass_settings.should_clear_depth = 1;

  wg_draw_pass_t* draw_pass = wg_create_draw_pass(wing, pipeline, draw_pass_settings);

  wg_command_t* command = wg_draw_pass_get_command(draw_pass);

  wg_resource_binding_t bindings[1] = { { 0, camera_uniform } };
  wg_vertex_buffer_t* vertex_buffers[2] = { position_buffer, color_buffer };

  wg_cmd_start_recording(command, wg_get_default_framebuffer(wing));
  wg_cmd_bind_resource_set(command, 0, 1, bindings);
  wg_cmd_draw(command, 2, vertex_buffers, index_buffer);
  wg_cmd_end_recording(command);

  wg_semaphore_t* image_ready_semaphore = wg_wingine_create_image_ready_semaphore(wing);
  wg_draw_pass_set_wait_semaphores(draw_pass, 1, &image_ready_semaphore);

  wg_semaphore_t* on_finish_semaphore = wg_draw_pass_create_on_finish_semaphore(draw_pass);
  wg_wingine_set_present_wait_semaphores(wing, 1, &on_finish_semaphore);

  float camera_matrix[16] = { 1.73205, 0, 0, 0,
                              0, -1.5396, 0, 0,
                              0, 0, -1.0001, -1,
                              0, 0, -0.010001, 0 };

  while (wg_wingine_is_window_open(wing)) {
    wg_uniform_set_current(camera_uniform, (void*)camera_matrix);

    wg_draw_pass_render(draw_pass);

    wg_wingine_present(wing);

    wg_wingine_sleep_milliseconds(wing, 40);

    wg_wingine_flush_events(wing);

    if (wg_wingine_is_key_pressed(wing, WK_ESC)) {
      break;
    }
  }

  wg_wingine_wait_idle(wing);

  wg_destroy_semaphore(on_finish_semaphore);
  wg_destroy_semaphore(image_ready_semaphore);

  wg_destroy_draw_pass(draw_pass);

  wg_destroy_pipeline(pipeline);

  wg_destroy_shader(vertex_shader);
  wg_destroy_shader(fragment_shader);

  wg_free_spv(vertex_spv);
  wg_free_spv(fragment_spv);

  wg_destroy_uniform(camera_uniform);

  wg_destroy_vertex_buffer(position_buffer);
  wg_destroy_vertex_buffer(color_buffer);
  wg_destroy_index_buffer(index_buffer);

  wg_destroy_wingine(wing);
}
