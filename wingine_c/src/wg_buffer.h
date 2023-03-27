#ifndef _WG_BUFFER
#define _WG_BUFFER

#include "./types.h"
#include "./wg_wingine.h"

EXTERNC

typedef struct wg_vertex_buffer_t {
  wg_sp_t *sp;
  void *vertex_buffer;
} wg_vertex_buffer_t;

int wg_create_vertex_buffer(wg_wingine_t *wing, wg_vertex_buffer_t *buffer, uint32_t byte_size);
int wg_set_vertex_buffer(wg_vertex_buffer_t *buffer,
                         void* data,
                         uint32_t byte_offset,
                         uint32_t byte_size);

int wg_destroy_vertex_buffer(wg_vertex_buffer_t *buffer);

struct wg_buffer_t {
  wg_sp_t *sp;
  void *buffer;
};

typedef struct wg_index_buffer_t {
  wg_sp_t *sp;
  void *index_buffer;
} wg_index_buffer_t;

int wg_create_index_buffer(wg_wingine_t *wing, wg_index_buffer_t *buffer, uint32_t num_indices);
int wg_set_index_buffer(wg_index_buffer_t *buffer, uint32_t* data, uint32_t first_index, uint32_t num_indices);
int wg_destroy_index_buffer(wg_index_buffer_t *index_buffer);

EXTERNC_END

#endif
