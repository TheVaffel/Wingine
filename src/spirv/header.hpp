#pragma once

#include "./ParseResult.hpp"
#include "./Operation.hpp"

namespace wg::spirv::header {
    void parseEntryPoint(Operation operation, ParseResult& parse_result);
}
