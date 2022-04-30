#pragma once

#include "./ManagedSemaphoreChain.hpp"

namespace wg::internal {

    enum class SemaphoreSetType {
        Signal,
        Wait
    };

    class SemaphoreSetBase {
    private:

        SemaphoreSetType type;
        void registerAllSemaphores() const;
        void unregisterAllSemaphores() const;

        std::vector<std::vector<vk::Semaphore>>
        createRawSemaphores(const std::vector<SemaphoreChainPtr>& semaphore_chains);
        vk::Semaphore extractRawSemaphore(SemaphoreChainPtr semaphore,
                                          uint32_t index) const;

    protected:
        uint32_t chain_length;
        IndexCounter raw_semaphores_index;

        std::shared_ptr<const DeviceManager> device_manager;

        std::vector<SemaphoreChainPtr> semaphore_chains;
        std::vector<std::vector<vk::Semaphore>> raw_semaphores;

        void init(const std::vector<SemaphoreChainPtr>& semaphore_chains);
        void initRawSemaphores();

        void registerSemaphore(SemaphoreChainPtr semaphore) const;
        void unregisterSemaphore(SemaphoreChainPtr semaphore) const;

        SemaphoreSetBase(SemaphoreSetType type,
                         uint32_t chain_length,
                         std::shared_ptr<const DeviceManager> device_manager);
        SemaphoreSetBase(SemaphoreSetType type,
                         const std::vector<SemaphoreChainPtr>& semaphores);

        SemaphoreSetBase(const SemaphoreSetBase& semaphore_set_base) = delete;

        virtual ~SemaphoreSetBase();

    public:

        SemaphoreSetBase& operator=(const SemaphoreSetBase& semaphore_set_base);

        const std::vector<vk::Semaphore>& getCurrentRawSemaphores() const;
        uint32_t getNumSemaphores() const;

        virtual void swapSemaphores() = 0;

        SemaphoreChainPtr addSemaphoreChain();
    };
};
