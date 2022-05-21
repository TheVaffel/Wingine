#pragma once

#include "./Wingine.hpp"

#include "./resource/BasicUniform.hpp"

namespace wg {
    template<typename T>
    Uniform<T> Wingine::createUniform() {
        return std::make_shared<internal::BasicUniform<T>>(this->staging_buffer_manager,
                                                           this->device_manager);
    }
};
