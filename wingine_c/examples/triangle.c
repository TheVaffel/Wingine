
#include "../wingine_c.h"

#include <stdint.h>

int main() {
  wg_wingine_t wing;

  const int width = 800, height = 800;

  wg_initialize_wingine(&wing, width, height);

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

  wg_vertex_buffer_t position_buffer;
  wg_create_vertex_buffer(&wing, &position_buffer, num_points * sizeof(float) * 4);
  wg_set_vertex_buffer(&position_buffer, positions, 0, num_points * sizeof(float) * 4);

  wg_vertex_buffer_t color_buffer;
  wg_create_vertex_buffer(&wing, &color_buffer, num_points * sizeof(float) * 4);
  wg_set_vertex_buffer(&color_buffer, colors, 0, num_points * sizeof(float) * 4);

  wg_index_buffer_t index_buffer;
  wg_create_index_buffer(&wing, &index_buffer, num_triangles * 3);
  wg_set_index_buffer(&index_buffer, indices, 0, num_triangles * 3);

  wg_uniform_chain_t camera_uniform;
  wg_create_uniform(&wing, &camera_uniform, 16 * sizeof(float));


  wg_destroy_uniform(&camera_uniform);

  wg_destroy_vertex_buffer(&position_buffer);
  wg_destroy_vertex_buffer(&color_buffer);
  wg_destroy_index_buffer(&index_buffer);

  wg_destroy_wingine(&wing);
}
