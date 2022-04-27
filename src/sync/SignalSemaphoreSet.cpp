#include "./SignalSemaphoreSet.hpp"

#include "./semaphoreUtil.hpp"

namespace wg::internal {

    namespace {
        uint32_t getChainLengthFromSemaphores(
            const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores) {
            if (semaphores.size() == 0) {
                throw std::runtime_error(
                    "[SignalSemaphoreSet] Cannot find chain length from zero-length semaphore list");
            }

            return std::begin(semaphores)[0]->getNumSemaphores();
        }

        std::shared_ptr<const DeviceManager>  getDeviceManagerFromSemaphores(
            const std::initializer_list<std::shared_ptr<ManagedSemaphoreChain>>& semaphores) {
            if (semaphores.size() == 0) {
                throw std::runtime_error(
                    "[SignalSemaphoreSet] Cannot find device manager from zero-length semaphore list");
            }

            return std::begin(semaphores)[0]->getDeviceManager();
        }

        std::vector<std::vector<vk::Semaphore>>
        createRawSignalSemaphoreChains(const std::vector<std::shared_ptr<ManagedSemaphoreChain>>& semaphore_chains) {
            if (semaphore_chains.size() == 0) {
                return std::vector<std::vector<vk::Semaphore>>();
            }

            std::vector<std::vector<vk::Semaphore>> chains(semaphore_chains[0]->getNumSemaphores());

            for (uint32_t i = 0; i < chains.size(); i++) {
                chains[i].reserve(semaphore_chains.size());

                for (uint32_t j = 0; j < semaphore_chains.size(); j++) {
                    chains[i].push_back(semaphore_chains[j]->getSemaphoreRelativeToCurrentSignal(i));
                }
            }

            return chains;
        }
    };

    SignalSemaphoreSet::SignalSemaphoreSet(uint32_t chain_length,
                                       std::shared_ptr<const DeviceManager> device_manager)
        : chain_length(chain_length),
          raw_semaphores_index(chain_length),
          device_manager(device_manager) { }


    SignalSemaphoreSet::SignalSemaphoreSet(const std::initializer_list<
                                       std::shared_ptr<ManagedSemaphoreChain>>& semaphores)
        : chain_length(getChainLengthFromSemaphores(semaphores)),
          raw_semaphores_index(chain_length),
          device_manager(getDeviceManagerFromSemaphores(semaphores)),
          semaphore_chains(semaphores),
          raw_semaphores(createRawSignalSemaphoreChains(semaphore_chains)) {

        for (std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->registerSignalSet();
        }
    }

    SignalSemaphoreSet& SignalSemaphoreSet::operator=(const SignalSemaphoreSet& semaphore_set) {
        for (std::shared_ptr<ManagedSemaphoreChain> semaphore_chain : this->semaphore_chains) {
            semaphore_chain->unregisterWaitSet();
        }

        this->semaphore_chains = semaphore_set.semaphore_chains;
        this->raw_semaphores = createRawSignalSemaphoreChains(this->semaphore_chains);

        for (std::shared_ptr<ManagedSemaphoreChain> semaphore_chain : this->semaphore_chains) {
            semaphore_chain->registerSignalSet();
        }

        return *this;
    }

    const std::vector<vk::Semaphore>& SignalSemaphoreSet::getCurrentRawSemaphores() const {
        return this->raw_semaphores[this->raw_semaphores_index.getCurrentIndex()];
    }

    uint32_t SignalSemaphoreSet::getNumSemaphores() const {
        return this->semaphore_chains.size();
    }

    std::shared_ptr<ManagedSemaphoreChain> SignalSemaphoreSet::addSemaphoreChain() {
        std::shared_ptr<ManagedSemaphoreChain> semaphore_chain =
            std::make_shared<ManagedSemaphoreChain>(this->chain_length,
                                                    this->device_manager);

        semaphore_chain->registerSignalSet();
        this->semaphore_chains.push_back(semaphore_chain);
        this->raw_semaphores = createRawSignalSemaphoreChains(this->semaphore_chains);

        return semaphore_chain;
    }

    std::shared_ptr<ManagedSemaphoreChain> SignalSemaphoreSet::addSignalledSemaphoreChain(const vk::Queue& queue) {
        std::shared_ptr<ManagedSemaphoreChain> semaphore_chain =
            std::make_shared<ManagedSemaphoreChain>(this->chain_length,
                                                    this->device_manager);

        semaphoreUtil::signalSemaphore(semaphore_chain->getSemaphoreRelativeToCurrentSignal(0),
                                       queue);
        semaphore_chain->markAsSignalled();

        semaphore_chain->registerSignalSet();

        this->semaphore_chains.push_back(semaphore_chain);
        this->raw_semaphores = createRawSignalSemaphoreChains(this->semaphore_chains);

        return semaphore_chain;
    }

    void SignalSemaphoreSet::swapSemaphores() {
        this->raw_semaphores_index.incrementIndex();

        for (std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->swapSignalSemaphore();
        }
    }

    SignalSemaphoreSet::~SignalSemaphoreSet() {
        for (std::shared_ptr<ManagedSemaphoreChain>& semaphore_chain : this->semaphore_chains) {
            semaphore_chain->unregisterSignalSet();
        }
    }
};
