#pragma once

#include "./enums.hpp"

namespace wg::spirv {

    struct Variable {
        StorageClass storage_class;
        uint32_t type_identifier;
        uint32_t identifier;
    };
};
