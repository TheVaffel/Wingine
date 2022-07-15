#include "./math.hpp"

namespace wg::internal::math {
    uint32_t roundUpMultiple(uint32_t value, uint32_t factor) {
        return ((value + factor - 1) / factor) * factor;
    }
    uint32_t roundDownMultiple(uint32_t value, uint32_t factor) {
        return (value / factor) * factor;;
    }
};
