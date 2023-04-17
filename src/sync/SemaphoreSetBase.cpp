#include "./SemaphoreSetBase.hpp"

#include <iostream>

namespace wg::internal {

    static const std::vector<vk::Semaphore> global_empty_raw_semaphores;

    namespace {
        uint32_t getChainLengthFromSemaphores(
            const std::vector<SemaphoreChainPtr>& semaphores) {
            if (semaphores.size() == 0) {
                /* throw std::runtime_error(
                   "[SemaphoreSetBase] Cannot find chain length from zero-length semaphore list"); */
                return 0;
            }

            return std::begin(semaphores)[0]->getNumSemaphores();
        }

        std::shared_ptr<const DeviceManager> getDeviceManagerFromSemaphores(
            const std::vector<SemaphoreChainPtr>& semaphores) {
            if (semaphores.size() == 0) {
                /* throw std::runtime_error(
                   "[SemaphoreSetBase] Cannot find device manager from zero-length semaphore list"); */
                return 0;
            }

            return std::begin(semaphores)[0]->getDeviceManager();
        }
    };

    SemaphoreSetBase::SemaphoreSetBase(SemaphoreSetType type,
                                       uint32_t chain_length,
                                       std::shared_ptr<const DeviceManager> device_manager)
        : type(type),
          chain_length(chain_length),
          raw_semaphores_index(chain_length),
          device_manager(device_manager) { }


    SemaphoreSetBase::SemaphoreSetBase(SemaphoreSetType type,
                                       const std::vector<SemaphoreChainPtr>& semaphores)
        : type(type),
          chain_length(getChainLengthFromSemaphores(semaphores)),
          raw_semaphores_index(chain_length),
          device_manager(getDeviceManagerFromSemaphores(semaphores)) {
        this->init(semaphores);
    }

    void SemaphoreSetBase::init(const std::vector<SemaphoreChainPtr>& semaphore_chains) {
        this->semaphore_chains = semaphore_chains;
        this->initRawSemaphores();
        this->registerAllSemaphores();
    }

    void SemaphoreSetBase::initRawSemaphores() {
        this->raw_semaphores = this->createRawSemaphores(this->semaphore_chains);
    }

    std::vector<std::vector<vk::Semaphore>>
    SemaphoreSetBase::createRawSemaphores(const std::vector<SemaphoreChainPtr>& semaphore_chains) {
        if (semaphore_chains.size() == 0) {
            return std::vector<std::vector<vk::Semaphore>>();
        }

        uint32_t chain_length = semaphore_chains[0]->getNumSemaphores();
        for (uint32_t i = 1; i < semaphore_chains.size(); i++) {
            if (semaphore_chains[i]->getNumSemaphores() != chain_length) {
                throw std::runtime_error("[SemaphoreSetBase] Semaphores chains in semaphore set has different lengths");
            }
        }

        if (this->raw_semaphores_index.getNumIndices() != chain_length) {
            this->raw_semaphores_index = IndexCounter(chain_length,
                                                      this->raw_semaphores_index.getCurrentIndex() % chain_length);
        }

        std::vector<std::vector<vk::Semaphore>> chains(semaphore_chains[0]->getNumSemaphores());

        for (uint32_t i = 0; i < chains.size(); i++) {
            chains[i].reserve(semaphore_chains.size());

            for (uint32_t j = 0; j < semaphore_chains.size(); j++) {
                chains[i].push_back(this->extractRawSemaphore(semaphore_chains[j], i));
            }
        }

        return chains;
    }

    void SemaphoreSetBase::registerAllSemaphores() const {
        for (SemaphoreChainPtr semaphore_chain : this->semaphore_chains) {
            this->registerSemaphore(semaphore_chain);
        }
    }

    void SemaphoreSetBase::unregisterAllSemaphores() const {
        for (SemaphoreChainPtr semaphore_chain : this->semaphore_chains) {
            this->unregisterSemaphore(semaphore_chain);
        }
    }


    void SemaphoreSetBase::registerSemaphore(SemaphoreChainPtr semaphore) const {
        switch (this->type) {
        case SemaphoreSetType::Signal:
            semaphore->registerSignalSet();
            break;
        case SemaphoreSetType::Wait:
            semaphore->registerWaitSet();
            break;
        }
    }

    void SemaphoreSetBase::unregisterSemaphore(SemaphoreChainPtr semaphore) const {
        switch (this->type) {
        case SemaphoreSetType::Signal:
            semaphore->unregisterSignalSet();
            break;
        case SemaphoreSetType::Wait:
            semaphore->unregisterWaitSet();
            break;
        }
    }

    vk::Semaphore SemaphoreSetBase::extractRawSemaphore(SemaphoreChainPtr semaphore,
                                                        uint32_t index) const {
        switch (this->type) {
        case SemaphoreSetType::Signal:
            return semaphore->getSemaphoreRelativeToCurrentSignal(index);
        case SemaphoreSetType::Wait:
            return semaphore->getSemaphoreRelativeToCurrentWait(index);
        default:
            throw std::runtime_error("[SemaphoreSetBase] Unhandled semaphore set type");
        }
    }


    const std::vector<vk::Semaphore>& SemaphoreSetBase::getCurrentRawSemaphores() const {
        if (this->raw_semaphores.size() == 0) {
            return global_empty_raw_semaphores;
        }
        return this->raw_semaphores[this->raw_semaphores_index.getCurrentIndex()];
    }

    uint32_t SemaphoreSetBase::getNumSemaphores() const {
        return this->semaphore_chains.size();
    }

    std::shared_ptr<ManagedSemaphoreChain> SemaphoreSetBase::addSemaphoreChain() {
        std::shared_ptr<ManagedSemaphoreChain> semaphore_chain =
            std::make_shared<ManagedSemaphoreChain>(this->chain_length,
                                                    this->device_manager);

        semaphore_chain->registerSignalSet();
        this->semaphore_chains.push_back(semaphore_chain);
        this->raw_semaphores = createRawSemaphores(this->semaphore_chains);

        return semaphore_chain;
    }

    void SemaphoreSetBase::adoptFrom(SemaphoreSetBase&& other_base) {
        this->unregisterAllSemaphores();
        this->removeSemaphores();

        other_base.unregisterAllSemaphores();

        this->init(other_base.semaphore_chains);

        other_base.removeSemaphores();
    }

    void SemaphoreSetBase::removeSemaphores() {
        this->semaphore_chains.clear();
        this->raw_semaphores.clear();
    }

    void SemaphoreSetBase::clear() {
        this->unregisterAllSemaphores();
        this->removeSemaphores();
    }

    SemaphoreSetBase::~SemaphoreSetBase() {
        this->unregisterAllSemaphores();
    }
};
