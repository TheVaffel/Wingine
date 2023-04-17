#include "./WrappedDSLCI.hpp"

namespace wg::internal {

    WrappedDSLCI::WrappedDSLCI(std::vector<vk::DescriptorSetLayoutBinding> bindings) {
        this->bindings = bindings;
        this->create_info.setBindings(bindings);
    }

    WrappedDSLCI::WrappedDSLCI(const WrappedDSLCI& other) {
        this->bindings = other.bindings;
        this->create_info = other.create_info;
        this->create_info.pBindings = this->bindings.data();
    }

    WrappedDSLCI& WrappedDSLCI::operator=(const WrappedDSLCI& other) {
        this->bindings = other.bindings;
        this->create_info = other.create_info;
        this->create_info.pBindings = this->bindings.data();

        return *this;
    }

    vk::DescriptorSetLayoutCreateInfo WrappedDSLCI::getCreateInfo() const {
        return this->create_info;
    }
};
