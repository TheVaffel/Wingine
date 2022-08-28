#include "./VertexAttribDesc.hpp"

namespace wg::internal {

    VertexAttribDesc::VertexAttribDesc(uint32_t binding_num,
                                       ComponentType component_type,
                                       uint32_t num_components,
                                       uint32_t stride_in_bytes,
                                       uint32_t offset_in_bytes,
                                       bool per_instance)
        : binding_num(binding_num),
          component_type(component_type),
          num_components(num_components),
          stride_in_bytes(stride_in_bytes),
          offset_in_bytes(offset_in_bytes),
          per_instance(per_instance)
    { }



    template<typename T>
    VertexAttribDesc VertexAttribDesc::fromType(uint32_t binding_num,
                                                uint32_t stride_in_bytes,
                                                uint32_t offset_in_bytes,
                                                bool per_instance) {
        VertexAttribDesc desc(binding_num,
                              DefaultComponentTypeMap<T>::value,
                              DefaultNumComponentsMap<T>::value,
                              stride_in_bytes == 0 ? sizeof(T) : stride_in_bytes,
                              offset_in_bytes,
                              per_instance);

        return desc;
    }

    template<>
    constexpr ComponentType DefaultComponentTypeMap<float>::value = ComponentType::Float32;

    template<>
    constexpr ComponentType DefaultComponentTypeMap<double>::value = ComponentType::Float64;

    template<>
    constexpr ComponentType DefaultComponentTypeMap<falg::Vec3>::value = ComponentType::Float32;

    template<>
    constexpr ComponentType DefaultComponentTypeMap<falg::Vec4>::value = ComponentType::Float32;


    template<>
    constexpr uint32_t DefaultNumComponentsMap<float>::value = 1;

    template<>
    constexpr uint32_t DefaultNumComponentsMap<double>::value = 1;

    template<>
    constexpr uint32_t DefaultNumComponentsMap<falg::Vec3>::value = 3;

    template<>
    constexpr uint32_t DefaultNumComponentsMap<falg::Vec4>::value = 4;
};
