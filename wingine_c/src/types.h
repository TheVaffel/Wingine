
#ifdef __cplusplus

#define EXTERNC extern "C" {
#define EXTERNC_END };

#else

#define EXTERNC
#define EXTERNC_END

#endif

EXTERNC

typedef struct wg_sp_t wg_sp_t;

typedef struct wg_wingine_t wg_wingine_t;

struct wg_framebuffer_t;
struct wg_framebuffer_chain_t;
struct wg_draw_pass_t;
struct wg_basic_draw_pass_settings_t;

struct wg_semaphore_t;
struct wg_wait_semaphore_set_t;
struct wg_signal_semaphore_set_t;
struct wg_event_chain_t;

struct wg_image_copier_t;
struct wg_image_chain_copier_t;

struct wg_resource_set_chain_t;
struct wg_uniform_t;
struct wg_uniform_chain_t;

struct wg_texture_t;
struct wg_settable_texture_t;
struct wg_storage_texture_t;
struct wg_texture_chain_t;
struct wg_framebuffer_texture_chain_t;

struct wg_shader_t;
struct wg_pipeline_t;
struct wg_compute_pipeline_t;
struct wg_basic_texture_setup_t;
struct wg_shader_stage_t;

struct wg_vertex_attrib_desc_t;
struct wg_component_type_t;
struct wg_vertex_buffer_t;
struct wg_buffer_t;
struct wg_index_buffer_t;
struct wg_storage_buffer_t;
struct wg_chain_reel_t;

EXTERNC_END
