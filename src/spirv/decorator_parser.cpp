#include "./decorator_parser.hpp"

#include <flawed_assert.hpp>

#include "./ParseResult.hpp"

namespace wg::spirv::decorator {
    void parseDecorator(Operation operation, ParseResult& parse_result) {

        fl_assert_eq(operation.getOpCode(), OpCode::Decorate);

        switch ((Decoration) operation.getArguments()[1]) {
        case Decoration::DescriptorSet:
            parse_result.descriptor_set_decorators.push_back(operation);
            break;
        case Decoration::Binding:
            std::cout << "Registered binding decoration on parse!! Had binding " << operation.getArg(2) << " and target " << operation.getArg(0) << std::endl;
            parse_result.descriptor_binding_decorators.push_back(operation);
            break;
        }
    }
};
