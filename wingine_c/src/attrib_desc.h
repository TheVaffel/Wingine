#include "./types.h"

EXTERNC

typedef enum wg_component_type {
  WG_COMPONENT_TYPE_FLOAT32,
  WG_COMPONENT_TYPE_FLOAT64,
  WG_COMPONENT_TYPE_INT32,
  WG_COMPONENT_TYPE_INT64
} wg_component_type;

typedef struct wg_vertex_attrib_desc_t {
  uint32_t binding_num;
  wg_component_type component_type;
  uint32_t num_components;
  uint32_t stride_in_bytes;
  uint32_t offset_in_bytes;
  uint8_t per_instance;
} wg_vertex_attrib_desc_t;

wg_vertex_attrib_desc_t wg_create_attrib_desc(uint32_t binding_num,
                                              wg_component_type component_type,
                                              uint32_t num_components,
                                              uint32_t stride_in_bytes,
                                              uint32_t offset_in_bytes);

wg_vertex_attrib_desc_t wg_create_instance_attrib_desc(uint32_t binding_num,
                                                       wg_component_type component_type,
                                                       uint32_t num_components,
                                                       uint32_t stride_in_bytes,
                                                       uint32_t offset_in_bytes);

EXTERNC_END
