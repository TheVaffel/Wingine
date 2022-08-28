#pragma once

#include <cstdint>

#include "./ComponentType.hpp"

namespace wg::internal {

    struct VertexAttribDesc {

        uint32_t binding_num;
        ComponentType component_type;
        uint32_t num_components;
        uint32_t stride_in_bytes;
        uint32_t offset_in_bytes;
        bool per_instance = false;

        VertexAttribDesc(uint32_t binding_num,
                         ComponentType component_type,
                         uint32_t num_components,
                         uint32_t stride_in_bytes,
                         uint32_t offset_in_bytes,
                         bool per_instance = false);

        template<typename T>
        static VertexAttribDesc fromType(uint32_t binding_num,
                                         uint32_t stride_in_bytes = 0,
                                         uint32_t offset_in_bytes = 0,
                                         bool per_instance = false);
    };



    template<typename T>
    struct DefaultComponentTypeMap {
        static const ComponentType value;
    };

    template<typename T>
    struct DefaultNumComponentsMap {
        static const uint32_t value;
    };
};
