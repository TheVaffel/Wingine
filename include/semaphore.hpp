#ifndef __WINGINE_SEMAPHORES_HPP
#define __WINGINE_SEMAPHORES_HPP

#include "declarations.hpp"

namespace wg {
  class SemaphoreChain {
    
    // semaphore will be a timeline semaphore. This is so that it can be used again and again
    // without the need to ensure that it has previously been unsignalled e.g.
    vk::Semaphore semaphore;
    int curr_pos;
    static const int initial_value = 0;

    SemaphoreChain(Wingine& wing);
    
    static int getWaitSemaphores(vk::Semaphore* arr, SemaphoreChain* const * objs, int count);
    static int getSignalSemaphores(vk::Semaphore* arr, SemaphoreChain* const * objs, int count);
    static int getSemaphoreWaitValues(uint64_t* values, SemaphoreChain* const * objs, int count);
    static int getSemaphoreSignalValues(uint64_t* values, SemaphoreChain* const * objs, int count);
    static int getWaitStages(vk::PipelineStageFlags* flags, SemaphoreChain* const * objs, int count);
    static void resetModifiers(SemaphoreChain* const * objs, int count);

    bool shouldBeSignalled() const;
    bool shouldBeWaitedUpon() const;
    
    bool isFirst() const;
    
    int getCurrVal() const;
    int getNextVal();
    vk::Semaphore getSemaphore() const;
    void resetModifiers(); // For erasing flags denoting use

    vk::PipelineStageFlagBits getFlag() const;

    static void chainsToSemaphore(Wingine* wing, SemaphoreChain* const * chains, int num_chains, vk::Semaphore semaphore);
    static void semaphoreToChains(Wingine* wing, vk::Semaphore semaphore, SemaphoreChain* const * chains, int num_chains);
    
    friend class Wingine;
    friend class RenderFamily;
  };
};

#endif // __WINGINE_SEMAPHORES_HPP
