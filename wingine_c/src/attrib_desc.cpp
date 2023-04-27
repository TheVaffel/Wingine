#include "./attrib_desc.h"

extern "C" {

    wg_vertex_attrib_desc_t wg_create_attrib_desc(uint32_t binding_num,
                                                  wg_component_type component_type,
                                                  uint32_t num_components,
                                                  uint32_t stride_in_bytes,
                                                  uint32_t offset_in_bytes) {
        return wg_vertex_attrib_desc_t {
            .binding_num = binding_num,
            .component_type = component_type,
            .num_components = num_components,
            .stride_in_bytes = stride_in_bytes,
            .offset_in_bytes = offset_in_bytes,
            .per_instance = 0
        };
    }

    wg_vertex_attrib_desc_t wg_create_instance_attrib_desc(uint32_t binding_num,
                                                           wg_component_type component_type,
                                                           uint32_t num_components,
                                                           uint32_t stride_in_bytes,
                                                           uint32_t offset_in_bytes) {
        return wg_vertex_attrib_desc_t {
            .binding_num = binding_num,
            .component_type = component_type,
            .num_components = num_components,
            .stride_in_bytes = stride_in_bytes,
            .offset_in_bytes = offset_in_bytes,
            .per_instance = 1
        };
    }

};
