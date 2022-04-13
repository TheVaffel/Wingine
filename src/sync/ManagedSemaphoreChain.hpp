#pragma once

#include "../DeviceManager.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>
#include <memory>

namespace wg::internal {
    class ManagedSemaphoreChain {

        uint32_t current_semaphore_index;

        std::vector<vk::Semaphore> semaphores;
        std::shared_ptr<const DeviceManager> device_manager;

    public:

        ManagedSemaphoreChain(uint32_t num_semaphores,
                              std::shared_ptr<const DeviceManager> device_manager);

        void swapSemaphore();

        const uint32_t getNumSemaphores() const;
        const vk::Semaphore getSemaphoreRelativeToCurrent(uint32_t index) const;

        ManagedSemaphoreChain(const ManagedSemaphoreChain&) = delete;

        ~ManagedSemaphoreChain();
    };
};
