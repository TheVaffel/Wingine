#include "./semaphoreUtil.hpp"

#include <iostream>

namespace wg::internal::semaphoreUtil {

    void signalManySemaphoresFromManySemaphores(const std::vector<vk::Semaphore>& wait_semaphores,
                                                const std::vector<vk::Semaphore>& signal_semaphores,
                                                const vk::Queue& queue) {
        std::vector<vk::PipelineStageFlags> pflags =
            std::vector<vk::PipelineStageFlags>(wait_semaphores.size(), vk::PipelineStageFlagBits::eTopOfPipe);

        vk::SubmitInfo info;
        info.setCommandBufferCount(0)
            .setWaitSemaphoreCount(wait_semaphores.size())
            .setPWaitSemaphores(wait_semaphores.data())
            .setPWaitDstStageMask(pflags.data())
            .setSignalSemaphores(signal_semaphores);

        _wassert_result(queue.submit(1, &info, nullptr),
                        "submit command to signal many from single semaphore");
    }

    void signalManySemaphoresFromSingleSemaphore(const std::vector<vk::Semaphore>& many_semaphores,
                                                 const vk::Semaphore& semaphore,
                                                 const vk::Queue& queue) {
        vk::PipelineStageFlags pflag = vk::PipelineStageFlagBits::eTopOfPipe;

        vk::SubmitInfo inf;
        inf.setCommandBufferCount(0)
            .setWaitSemaphoreCount(1)
            .setPWaitDstStageMask(&pflag)
            .setPWaitSemaphores(&semaphore)
            .setSignalSemaphoreCount(many_semaphores.size())
            .setPSignalSemaphores(many_semaphores.data());

        _wassert_result(queue.submit(1, &inf, nullptr),
                        "submit command to signal many from single semaphore");

    }

    void signalSemaphore(const vk::Semaphore& semaphore,
                         const vk::Queue& queue) {
        signalSemaphores({semaphore}, queue);
    }


    void signalSemaphore(const std::vector<vk::Semaphore>& semaphores,
                         const vk::Queue& queue) {

        vk::SubmitInfo inf;
        inf.setCommandBufferCount(0)
            .setWaitSemaphoreCount(0)
            .setSignalSemaphores(semaphores);

        _wassert_result(queue.submit(1, &inf, nullptr),
                        "submit command to signal single semaphore");
    }
}
