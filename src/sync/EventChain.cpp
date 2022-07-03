#include "./EventChain.hpp"

namespace wg::internal {


    EventChain::EventChain(uint32_t count,
                           std::shared_ptr<DeviceManager> device_manager)
        : event_index(count), device_manager(device_manager) {

        events.resize(count);

        vk::EventCreateInfo eci;
        // eci.setFlags(vk::EventCreateFlagBits::eDeviceOnly);

        for (uint32_t i = 0; i < count; i++) {
            this->events[i] = device_manager->getDevice().createEvent(eci);
        }
    }

    vk::Event EventChain::getEventAt(uint32_t index) const {
        return this->events[index];
    }

    vk::Event EventChain::getCurrentEvent() const {
        return this->events[this->event_index.getCurrentIndex()];
    }

    void EventChain::swap() {
        this->event_index.incrementIndex();
    }

    void EventChain::resetCurrent() {
        this->device_manager->getDevice().resetEvent(this->events[this->event_index.getCurrentIndex()]);
    }

    EventChain::~EventChain() {
        for (uint32_t i = 0; i < this->event_index.getNumIndices(); i++) {
            this->device_manager->getDevice().destroy(this->events[i]);
        }
    }
};
