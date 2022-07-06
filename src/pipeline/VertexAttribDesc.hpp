#pragma once

#include <cstdint>

#include "./ComponentType.hpp"

namespace wg::internal {

    struct VertexAttribDesc {

        uint32_t binding_num;
        ComponentType component_type;
        uint32_t num_elements;
        uint32_t stride_in_bytes;
        uint32_t offset_in_bytes;
        bool per_instance = false;

        VertexAttribDesc(uint32_t binding_num,
                         ComponentType component_type,
                         uint32_t num_elements,
                         uint32_t stride_in_bytes,
                         uint32_t offset_in_bytes,
                         bool per_instance = false);
    };
};
