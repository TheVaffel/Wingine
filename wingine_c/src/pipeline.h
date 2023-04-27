#include "./types.h"

EXTERNC

typedef struct wg_pipeline_t wg_pipeline_t;

wg_pipeline_t* wg_create_pipeline(wg_wingine_t* wing,
                        wg_vertex_attrib_desc_t*,
                        uint32_t num_attrib_desc,
                        wg_shader_t** shaders,
                        uint32_t num_shaders);

void wg_destroy_pipeline(wg_pipeline_t* pipeline);

EXTERNC_END
