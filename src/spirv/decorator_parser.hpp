#pragma once

#include "./Operation.hpp"
#include "./ParseResult.hpp"

namespace wg::spirv::decorator {
    void parseDecorator(Operation operation, ParseResult& parse_result);
};
