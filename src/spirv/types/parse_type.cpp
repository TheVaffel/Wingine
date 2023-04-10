#include "./parse_type.hpp"

#include "./Type.hpp"
#include "./PointerType.hpp"

#include <iostream>

namespace wg::spirv::type {
    void parsePointerType(Operation operation, ParseResult& parse_result) {
        PointerType pt;
        pt.identifier = operation.getArg(0);
        pt.type_identifier = operation.getArg(2);
        pt.storage_class = (StorageClass) operation.getArg(1);

        parse_result.pointer_types[pt.identifier] = pt;
    }

    void parseBaseType(Operation operation, ParseResult& parse_result) {
        Type t;
        t.identifier = operation.getArg(0);
        t.base_type = (BaseType) operation.getOpCode();

        parse_result.base_types[t.identifier] = t;
    }
};
