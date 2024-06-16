#pragma once

#include "./types.h"

#include <Wingine.hpp>
#include <Winval.hpp>

struct win_winval_t {
    Winval winval;
};

struct wg_wingine_t {
    wg::Wingine wingine;
};

struct wg_vertex_buffer_t {
    wg::VertexBufferPtr<uint8_t> v;
};

struct wg_index_buffer_t {
    wg::IndexBufferPtr v;
};

struct wg_pipeline_t {
    wg::PipelinePtr v;
};

struct wg_shader_t {
    wg::ShaderPtr v;
};

struct wg_uniform_t {
    wg::RawUniformChainPtr v;
};

struct wg_draw_pass_t {
    wg::DrawPassPtr v;
};

struct wg_framebuffer_t {
    wg::FramebufferChain v;
};

struct wg_semaphore_t {
    wg::Semaphore v;
};
