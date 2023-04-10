#pragma once

#include "../Operation.hpp"
#include "../ParseResult.hpp"

namespace wg::spirv::type {
    struct PointerType {
        uint32_t identifier;
        uint32_t type_identifier;
        StorageClass storage_class;
    };
};
