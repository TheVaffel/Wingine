#include "./IndexCounter.hpp"

#include <flawed_assert.hpp>

namespace wg::internal {
    IndexCounter::IndexCounter(uint32_t num_indices)
        : num_indices(num_indices), current_index(0) { }

    IndexCounter::IndexCounter(uint32_t num_indices, uint32_t start_index)
        : num_indices(num_indices), current_index(start_index) {
        fl_assert_lt(start_index, num_indices);
    }

    uint32_t IndexCounter::getCurrentIndex() const {
        return this->current_index;
    }

    uint32_t IndexCounter::getNextIndex() const {
        return (this->current_index + 1) % this->num_indices;
    }

    uint32_t IndexCounter::getPreviousIndex() const {
        return (this->current_index + this->num_indices - 1) % this->num_indices;
    }

    uint32_t IndexCounter::getRelativeIndex(uint32_t offset) const {
        return (this->current_index + offset) % this->num_indices;
    }

    uint32_t IndexCounter::getNumIndices() const {
        return this->num_indices;
    }

    void IndexCounter::incrementIndex() {
        this->current_index = this->getNextIndex();
    }

    void IndexCounter::decrementIndex() {
        this->current_index = this->getPreviousIndex();
    }
};
