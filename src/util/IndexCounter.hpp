#pragma once

#include <cstdint>

namespace wg::internal {
    class IndexCounter {
        uint32_t num_indices;
        uint32_t current_index;
    public:

        IndexCounter(uint32_t num_indices);

        uint32_t getCurrentIndex() const;
        uint32_t getNextIndex() const;
        uint32_t getPreviousIndex() const;

        void incrementIndex();
        void decrementIndex();
    };
};
