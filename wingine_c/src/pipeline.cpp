#include "./pipeline.h"

#include <Wingine.hpp>

#include "./wingine.h"
#include "./shader.h"
#include "./attrib_desc.h"

#include "./types.hpp"

extern "C" {

    wg_pipeline_t* wg_create_pipeline(wg_wingine_t* wing,
                                      uint32_t num_attrib_desc,
                                      wg_vertex_attrib_desc_t* raw_descs,
                                      uint32_t num_shaders,
                                      wg_shader_t** raw_shaders) {
        std::vector<wg::ShaderPtr> shaders;
        for (uint32_t i = 0; i < num_shaders; i++) {
            shaders.push_back(raw_shaders[i]->v);
        }

        std::vector<wg::VertexAttribDesc> descs;

        for (uint32_t i = 0; i < num_attrib_desc; i++) {
            wg::VertexAttribDesc desc(raw_descs[i].binding_num,
                                      (wg::ComponentType)raw_descs[i].component_type,
                                      raw_descs[i].num_components,
                                      raw_descs[i].stride_in_bytes,
                                      raw_descs[i].offset_in_bytes);
            descs.push_back(desc);
        }

        return new wg_pipeline_t {
            .v = wing->wingine.createBasicPipeline(descs, shaders)
        };
    }

    void wg_destroy_pipeline(wg_pipeline_t* pipeline) {
        delete pipeline;
    }
};
