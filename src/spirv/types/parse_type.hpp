#pragma once

#include "../Operation.hpp"
#include "../ParseResult.hpp"

namespace wg::spirv::type {
    void parseBaseType(Operation operation, ParseResult& parse_result);
    void parsePointerType(Operation operation, ParseResult& parse_result);
};
