#pragma once

#include "./IndexCounter.hpp"

namespace wg::internal {

    class SettableIndexCounter : public IndexCounter {
    public:
        SettableIndexCounter(uint32_t num_indices);

        void setIndex(uint32_t index);
    };
};
