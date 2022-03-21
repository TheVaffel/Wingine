#ifndef __WINGINE_SEMAPHORES_HPP
#define __WINGINE_SEMAPHORES_HPP

#include "declarations.hpp"

namespace wg {
    class SemaphoreChain {

        // semaphore will be a timeline semaphore. This is so that it can be used again and again
        // without the need to ensure that it has previously been unsignalled e.g.
        vk::Semaphore semaphore;
        uint64_t curr_pos;
        static const int initial_value = 0;

        SemaphoreChain(Wingine& wing);

        static int getWaitSemaphores(vk::Semaphore* arr, SemaphoreChain* const * objs, int count);
        static int getSignalSemaphores(vk::Semaphore* arr, SemaphoreChain* const * objs, int count);
        static int getSemaphoreWaitValues(uint64_t* values, SemaphoreChain* const * objs, int count);
        static int getSemaphoreSignalValues(uint64_t* values, SemaphoreChain* const * objs, int count);
        static int getWaitStages(vk::PipelineStageFlags* flags, SemaphoreChain* const * objs, int count);

        void ensure_finished(Wingine* wing, const vk::Fence& fence);

        bool shouldBeSignalled() const;
        bool shouldBeWaitedUpon() const;

        bool isFirst() const;

        int getCurrVal() const;
        int getNextVal();
        vk::Semaphore getSemaphore() const;
        void resetModifiers(); // For erasing flags denoting use

        vk::PipelineStageFlagBits getFlag() const;
    public:
        static void chainsToSemaphore(const vk::Queue& queue, SemaphoreChain* const * chains, int num_chains, vk::Semaphore semaphore);
        static void semaphoreToChains(const vk::Queue& queue, vk::Semaphore semaphore, SemaphoreChain* const * chains, int num_chains);
        static void resetModifiers(SemaphoreChain* const * objs, int count);

        friend class Wingine;
        friend class RenderFamily;
    };
};

#endif // __WINGINE_SEMAPHORES_HPP
