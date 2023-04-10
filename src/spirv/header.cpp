#include "./header.hpp"
#include "./OpCode.hpp"

#include <flawed_assert.hpp>

namespace wg::spirv::header {
    void parseEntryPoint(Operation operation, ParseResult& parse_result) {
        fl_assert_eq(operation.getOpCode(), OpCode::EntryPoint);

        parse_result.execution_model = (ExecutionModel) operation.getArguments()[0];
        std::cout << "Found execution model = " << (uint32_t) parse_result.execution_model << std::endl;
    }
};
