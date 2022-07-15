#pragma once

#include <cstdint>

namespace wg::internal::math {
    uint32_t roundUpMultiple(uint32_t value, uint32_t factor);
    uint32_t roundDownMultiple(uint32_t value, uint32_t factor);
};
