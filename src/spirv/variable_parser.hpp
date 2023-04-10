#pragma once

#include "./Operation.hpp"
#include "./ParseResult.hpp"

namespace wg::spirv::variable {
    void parseVariable(Operation operation, ParseResult& parse_result);
};
