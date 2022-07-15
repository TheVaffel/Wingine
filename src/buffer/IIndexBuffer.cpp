#include "./IIndexBuffer.hpp"

namespace wg::internal {
    uint32_t IIndexBuffer::getNumIndices() const {
        return this->getByteSize() / sizeof(uint32_t);
    }
};
