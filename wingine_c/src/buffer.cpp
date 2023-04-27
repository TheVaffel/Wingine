#include "./buffer.h"
#include <Wingine.hpp>

#include "./types.hpp"

extern "C" {
    wg_vertex_buffer_t* wg_create_vertex_buffer(wg_wingine_t *wing, uint32_t byte_size) {
        return new wg_vertex_buffer_t {
            .v = wing->wingine.createVertexBuffer<uint8_t>(byte_size)
        };
    }

    void wg_set_vertex_buffer(wg_vertex_buffer_t *buffer,
                             void* data,
                             uint32_t byte_offset,
                             uint32_t byte_size) {
        buffer->v->set((uint8_t*)data, byte_offset, byte_size);
    }

    void wg_destroy_vertex_buffer(wg_vertex_buffer_t *buffer) {
        delete buffer;
    }


    wg_index_buffer_t* wg_create_index_buffer(wg_wingine_t *wing, uint32_t num_indices) {

        return new wg_index_buffer_t {
            .v = wing->wingine.createIndexBuffer(num_indices)
        };
    }

    void wg_set_index_buffer(wg_index_buffer_t *buffer, uint32_t* data, uint32_t first_index, uint32_t num_indices) {
        buffer->v->set(data, first_index, num_indices);
    }

    void wg_destroy_index_buffer(wg_index_buffer_t *index_buffer) {
        delete index_buffer;
    }
};
