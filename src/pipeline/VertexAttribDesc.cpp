#include "./VertexAttribDesc.hpp"

namespace wg::internal {

    VertexAttribDesc::VertexAttribDesc(uint32_t binding_num,
                                       ComponentType component_type,
                                       uint32_t num_elements,
                                       uint32_t stride_in_bytes,
                                       uint32_t offset_in_bytes,
                                       bool per_instance)
        : binding_num(binding_num),
          component_type(component_type),
          num_elements(num_elements),
          stride_in_bytes(stride_in_bytes),
          offset_in_bytes(offset_in_bytes),
          per_instance(per_instance)
    { }
};
