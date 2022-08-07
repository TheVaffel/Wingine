#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/DeviceManager.hpp"
#include "../util/IndexCounter.hpp"

namespace wg::internal {
    class EventChain {
        std::vector<vk::Event> events;

        IndexCounter event_index;

        std::shared_ptr<DeviceManager> device_manager;

    public:

        EventChain(uint32_t count,
                   std::shared_ptr<DeviceManager> device_manager);

        vk::Event getEventAt(uint32_t index) const;
        vk::Event getCurrentEvent() const;
        void swap();

        void resetCurrent();

        ~EventChain();
    };
};
