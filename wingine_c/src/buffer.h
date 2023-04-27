#ifndef _WG_BUFFER
#define _WG_BUFFER

#include "./types.h"
#include "./wingine.h"

EXTERNC

typedef struct wg_vertex_buffer_t wg_vertex_buffer_t;

wg_vertex_buffer_t* wg_create_vertex_buffer(wg_wingine_t *wing, uint32_t byte_size);
void wg_set_vertex_buffer(wg_vertex_buffer_t *buffer,
                          void* data,
                          uint32_t byte_offset,
                          uint32_t byte_size);

void wg_destroy_vertex_buffer(wg_vertex_buffer_t *buffer);

typedef struct wg_index_buffer_t wg_index_buffer_t;

wg_index_buffer_t* wg_create_index_buffer(wg_wingine_t *wing, uint32_t num_indices);
void  wg_set_index_buffer(wg_index_buffer_t *buffer, uint32_t* data, uint32_t first_index, uint32_t num_indices);
void wg_destroy_index_buffer(wg_index_buffer_t *index_buffer);

EXTERNC_END

#endif
