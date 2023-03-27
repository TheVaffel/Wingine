#include "./wg_buffer.h"
#include <Wingine.hpp>

extern "C" {
    int wg_create_vertex_buffer(wg_wingine_t *wing, wg_vertex_buffer_t *buffer, uint32_t byte_size) {
        auto sp = new wg::VertexBufferPtr<uint8_t>
            (((wg::Wingine*)wing->wingine)->createVertexBuffer<uint8_t>(byte_size));

        buffer->sp = (wg_sp_t*)sp;
        buffer->vertex_buffer = sp->get();
        return 0;
    }

    int wg_set_vertex_buffer(wg_vertex_buffer_t *buffer,
                             void* data,
                             uint32_t byte_offset,
                             uint32_t byte_size) {
        ((wg::internal::IVertexBuffer<uint8_t>*)buffer->vertex_buffer)->set((uint8_t*)data, byte_offset, byte_size);
        return 0;
    }

    int wg_destroy_vertex_buffer(wg_vertex_buffer_t *buffer) {
        delete (wg::VertexBufferPtr<uint8_t>*)buffer->sp;
        return 0;
    }


    int wg_create_index_buffer(wg_wingine_t *wing, wg_index_buffer_t *buffer, uint32_t num_indices) {
        auto sp = new wg::IndexBufferPtr(((wg::Wingine*)wing->wingine)->createIndexBuffer(num_indices));

        buffer->sp = (wg_sp_t*)sp;
        buffer->index_buffer = sp->get();
        return 0;
    }

    int wg_set_index_buffer(wg_index_buffer_t *buffer, uint32_t* data, uint32_t first_index, uint32_t num_indices) {
        ((wg::internal::IIndexBuffer*)buffer->index_buffer)->set(data, first_index, num_indices);
        return 0;
    }

    int wg_destroy_index_buffer(wg_index_buffer_t *index_buffer) {
        delete (wg::IndexBufferPtr*)index_buffer->sp;
        return 0;
    }
};
