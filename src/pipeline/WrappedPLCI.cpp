#include "./WrappedPLCI.hpp"

#include <iostream>

namespace wg::internal {

    WrappedPLCI::WrappedPLCI(vk::PipelineLayoutCreateInfo info,
                             const std::vector<vk::DescriptorSetLayout>& layouts) {
        this->info = info;
        this->layouts = layouts;
        this->info.setLayoutCount = this->layouts.size();
        this->info.pSetLayouts = this->layouts.data();
    }

    WrappedPLCI::WrappedPLCI(const WrappedPLCI& other) {
        this->info = other.info;
        this->layouts = other.layouts;
        this->info.pSetLayouts = this->layouts.data();
    }

    WrappedPLCI& WrappedPLCI::operator=(const WrappedPLCI& other) {
        this->info = other.info;
        this->layouts = other.layouts;
        this->info.pSetLayouts = this->layouts.data();

        return *this;
    }

    vk::PipelineLayoutCreateInfo WrappedPLCI::getCreateInfo() const {
        return this->info;
    }


    const std::vector<vk::DescriptorSetLayout>& WrappedPLCI::getLayouts() const {
        return this->layouts;
    }
};
