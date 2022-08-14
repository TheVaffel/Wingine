#pragma once

#include "./Wingine.hpp"

#include "./resource/BasicUniform.hpp"
#include "./resource/BasicUniformChain.hpp"
#include "./buffer/InternallyStagedVertexBuffer.hpp"
#include "./buffer/InternallyStagedStorageBuffer.hpp"
#include "./resource/BasicResourceSetChain.hpp"

namespace wg {
    template<typename T>
    UniformPtr<T> Wingine::createUniform() {
        return std::make_shared<internal::BasicUniform<T>>(this->device_manager);
    }

    template<typename T>
    UniformChainPtr<T> Wingine::createUniformChain() {
        auto uniform_chain = std::make_shared<internal::BasicUniformChain<T>>(this->getNumFramebuffers(),
                                                                              this->device_manager);
        this->current_chain_reel->addChain(uniform_chain);
        return uniform_chain;
    }

    template<typename T>
    VertexBufferPtr<T> Wingine::createVertexBuffer(uint32_t element_count) {
        return std::make_shared<internal::InternallyStagedVertexBuffer<T>>(element_count,
                                                                           this->device_manager,
                                                                           this->queue_manager,
                                                                           this->command_manager);
    }

    template <typename T>
    StorageBufferPtr<T> Wingine::createStorageBuffer(uint32_t element_count) {
        return std::make_shared<internal::InternallyStagedStorageBuffer<T>>(element_count,
                                                                            this->device_manager,
                                                                            this->queue_manager,
                                                                            this->command_manager);
    }

    template<typename... Ts>
    ResourceSetChainPtr Wingine::createResourceSetChain(const std::vector<uint64_t>& resourceLayout,
                                                        Ts... resources) {
        ResourceSetChainPtr res = std::make_shared<internal::BasicResourceSetChain>(
            this->getNumFramebuffers(),
            this->resource_set_layout_registry->ensureAndGet(resourceLayout),
            this->descriptor_pool,
            this->device_manager,
            resources...);
        this->current_chain_reel->addChain(res);
        return res;
    }
};
