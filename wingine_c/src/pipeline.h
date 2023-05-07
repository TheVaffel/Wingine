#ifndef _WG_PIPELINE
#define _WG_PIPELINE

#include "./types.h"
#include "./attrib_desc.h"
#include "./shader.h"

EXTERNC

typedef struct wg_pipeline_t wg_pipeline_t;

wg_pipeline_t* wg_create_pipeline(wg_wingine_t* wing,
                                  uint32_t num_attrib_desc,
                                  const wg_vertex_attrib_desc_t*,
                                  uint32_t num_shaders,
                                  wg_shader_t** shaders);

void wg_destroy_pipeline(wg_pipeline_t* pipeline);

EXTERNC_END

#endif
