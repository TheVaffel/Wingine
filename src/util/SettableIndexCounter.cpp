#include "./SettableIndexCounter.hpp"

#include <stdexcept>

namespace wg::internal {
    SettableIndexCounter::SettableIndexCounter(uint32_t num_indices) : IndexCounter(num_indices) { }

    void SettableIndexCounter::setIndex(uint32_t index) {
        if (index >= this->num_indices) {
            throw std::runtime_error("[SettableIndexCounter] Set index is too high");
        }
        this->current_index = index;
    }
};
