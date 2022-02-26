#include "./semaphore.hpp"
#include "./Wingine.hpp"

#include "./log.hpp"

namespace wg {

    SemaphoreChain::SemaphoreChain(Wingine& wing) {
        this->curr_pos = SemaphoreChain::initial_value;

        vk::SemaphoreCreateInfo sci;

        // Create timeline semaphore with initial value 0
        vk::SemaphoreTypeCreateInfo stci;
        stci.setSemaphoreType(vk::SemaphoreType::eTimeline)
            .setInitialValue(this->curr_pos);

        sci.setPNext(&stci);

        this->semaphore = wing.device.createSemaphore(sci);
    }

    int SemaphoreChain::getWaitSemaphores(vk::Semaphore* arr, SemaphoreChain* const* chains, int count) {
        int j = 0;
        for(int i = 0; i < count; i++) {
            if(!chains[i]->shouldBeWaitedUpon()) {
                continue;
            }

            arr[j++] = chains[i]->getSemaphore();
        }

        return j;
    }

    int SemaphoreChain::getSignalSemaphores(vk::Semaphore* arr, SemaphoreChain* const* chains, int count) {
        int j = 0;
        for(int i = 0; i < count; i++) {
            if(!chains[i]->shouldBeSignalled()) {
                continue;
            }
            arr[j++] = chains[i]->getSemaphore();
        }

        return j;
    }

    int SemaphoreChain::getSemaphoreWaitValues(uint64_t* values, SemaphoreChain* const* chains, int count) {
        int j = 0;
        for(int i = 0; i < count; i++) {
            if(!chains[i]->shouldBeWaitedUpon()) {
                continue;
            }
            values[j++] = chains[i]->getCurrVal();
        }

        return j;
    }

    int SemaphoreChain::getSemaphoreSignalValues(uint64_t* values, SemaphoreChain* const* chains, int count) {
        int j = 0;
        for(int i = 0; i < count; i++) {
            if(!chains[i]->shouldBeSignalled()) {
                continue;
            }
            values[j++] = chains[i]->getNextVal();
        }

        return j;
    }

    int SemaphoreChain::getWaitStages(vk::PipelineStageFlags* flags, SemaphoreChain* const * chains, int count) {
        int j = 0;
        for(int i = 0; i < count; i++) {
            if(!chains[i]->shouldBeWaitedUpon()) {
                continue;
            }
            flags[j++] = chains[i]->getFlag();
        }

        return j;
    }

    void SemaphoreChain::resetModifiers(SemaphoreChain* const * chains, int count) {
        for(int i = 0; i < count; i++) {
            chains[i]->resetModifiers();
        }
    }

    vk::PipelineStageFlagBits SemaphoreChain::getFlag() const {
        return vk::PipelineStageFlagBits::eTopOfPipe;
    }

    void SemaphoreChain::resetModifiers() {
        // Something will come here...
    }

    bool SemaphoreChain::isFirst() const {
        return this->curr_pos == SemaphoreChain::initial_value;
    }

    void SemaphoreChain::ensure_finished(Wingine* wing, const vk::Fence& fence) {
        vk::PipelineStageFlags flag = this->getFlag();

        vk::TimelineSemaphoreSubmitInfo tssi;
        tssi.setWaitSemaphoreValueCount(1)
            .setPWaitSemaphoreValues(&this->curr_pos);

        vk::SubmitInfo inf;
        inf.setCommandBufferCount(0)
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&this->semaphore)
            .setPWaitDstStageMask(&flag)
            .setPNext(&tssi);

        _wassert_result(wing->getPresentQueue().submit(1, &inf, fence),
                        "submit command in SemaphoreChain::ensure_finished");

        _wassert_result(wing->getDevice().waitForFences(1, &fence, true, UINT64_MAX),
                        "wait for command finish in SemaphoreChain::ensure_finished");
    }

    bool SemaphoreChain::shouldBeWaitedUpon() const {
        return !this->isFirst();
    }

    bool SemaphoreChain::shouldBeSignalled() const {
        return true; // Right now, there is no reason why we would not want it to be signalled
    }

    int SemaphoreChain::getNextVal() {
        this->curr_pos++;
        return this->curr_pos;
    }

    int SemaphoreChain::getCurrVal() const {
        return this->curr_pos;
    }

    vk::Semaphore SemaphoreChain::getSemaphore() const {
        return this->semaphore;
    }

    // Make empty command, causing signalling of multiple semaphore chains signal a single normal semaphore
    void SemaphoreChain::chainsToSemaphore(Wingine* wing, SemaphoreChain* const* chains, int num_chains, vk::Semaphore semaphore) {
        std::vector<vk::PipelineStageFlags> pflags(num_chains);
        std::vector<vk::Semaphore> semarr(num_chains);
        std::vector<uint64_t> wait_values(num_chains);

        int num_sems = SemaphoreChain::getWaitSemaphores(semarr.data(), chains, num_chains);
        int num_vals = SemaphoreChain::getSemaphoreWaitValues(wait_values.data(), chains, num_chains);
        int num_stages = SemaphoreChain::getWaitStages(pflags.data(), chains, num_chains);

        _wassert(num_sems == num_vals && num_vals == num_stages);

        vk::TimelineSemaphoreSubmitInfo tssi;
        tssi.setWaitSemaphoreValueCount(num_vals)
            .setPWaitSemaphoreValues(wait_values.data());

        vk::SubmitInfo inf;
        inf.setCommandBufferCount(0)
            .setWaitSemaphoreCount(num_sems)
            .setPWaitDstStageMask(pflags.data())
            .setPWaitSemaphores(semarr.data())
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(&semaphore)
            .setPNext(&tssi);

        _wassert_result(wing->getPresentQueue().submit(1, &inf, nullptr),
                        "submit command in SemaphoreChain::chainsToSemaphore");
    }

    // Make empty command, causing signalling a single semaphore to signal multiple semaphore chains
    void SemaphoreChain::semaphoreToChains(Wingine* wing, vk::Semaphore semaphore, SemaphoreChain* const* chains, int num_chains) {
        std::vector<vk::Semaphore> semarr(num_chains);
        vk::PipelineStageFlags pflag = vk::PipelineStageFlagBits::eTopOfPipe;

        std::vector<uint64_t> signal_values(num_chains);

        int num_sems = SemaphoreChain::getSignalSemaphores(semarr.data(), chains, num_chains);
        int num_vals = SemaphoreChain::getSemaphoreSignalValues(signal_values.data(), chains, num_chains);

        _wassert(num_sems == num_vals);

        vk::TimelineSemaphoreSubmitInfo tssi;
        tssi
            .setSignalSemaphoreValueCount(num_vals)
            .setPSignalSemaphoreValues(signal_values.data());

        vk::SubmitInfo inf;
        inf.setCommandBufferCount(0)
            .setWaitSemaphoreCount(1)
            .setPWaitDstStageMask(&pflag)
            .setPWaitSemaphores(&semaphore)
            .setSignalSemaphoreCount(num_sems)
            .setPSignalSemaphores(semarr.data())
            .setPNext(&tssi);

        // Present queue is hopefully not that busy
        _wassert_result(wing->getPresentQueue().submit(1, &inf, nullptr),
                        "submit command in SemaphoreChain::semaphoreToChains");
    }

};
