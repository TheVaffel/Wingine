#include "./variable_parser.hpp"
#include "./Operation.hpp"

#include "./Variable.hpp"

#include <flawed_assert.hpp>

namespace wg::spirv::variable {
    void parseVariable(Operation operation, ParseResult& parse_result) {
        fl_assert_eq(operation.getOpCode(), OpCode::Variable);

        Variable var;
        var.storage_class = (StorageClass) operation.getArg(2);
        var.type_identifier = operation.getArg(0);
        var.identifier = operation.getArg(1);

        parse_result.variables[var.identifier] = var;
    }
};
