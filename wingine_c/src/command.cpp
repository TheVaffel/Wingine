#include "./command.h"

#include "./types.hpp"
#include <Wingine.hpp>

struct generic_resource_t {
    wg::ResourceChainPtr resource;
};

extern "C" {

    wg_command_t* wg_draw_pass_get_command(wg_draw_pass_t *draw_pass) {
        return (wg_command_t*)&(draw_pass->v->getCommandChain());
    }

    void wg_cmd_start_recording(wg_command_t* command, wg_framebuffer_t* framebuffer) {
        wg::CommandChainController& c = *(wg::CommandChainController*)command;

        c.startRecording(*(wg::FramebufferChain*)framebuffer);
    }

    void wg_cmd_bind_resource_set(wg_command_t* command, uint32_t binding_index, uint32_t num_bindings, wg_resource_binding_t *raw_bindings) {
        wg::CommandChainController& c = *(wg::CommandChainController*)command;

        std::vector<wg::ResourceBinding> bindings;
        for (uint32_t i = 0; i < num_bindings; i++) {
            bindings.push_back({ raw_bindings[i].binding, ((generic_resource_t*)(raw_bindings[i].resource_binding))->resource });
        }

        c.recordBindResourceSet(binding_index, bindings);
    }

    void wg_cmd_draw(wg_command_t* command, uint32_t num_buffers, wg_vertex_buffer_t **raw_buffers, wg_index_buffer_t *index_buffer) {
        wg::CommandChainController& c = *(wg::CommandChainController*)command;

        std::vector<wg::BufferPtr> vertex_buffers;
        for (uint32_t i = 0; i < num_buffers; i++) {
            vertex_buffers.push_back(raw_buffers[i]->v);
        }

        c.recordDraw(vertex_buffers, index_buffer->v);
    }

    void wg_cmd_end_recording(wg_command_t *command) {
        wg::CommandChainController& c = *(wg::CommandChainController*)command;
        c.endRecording();
    }
};
