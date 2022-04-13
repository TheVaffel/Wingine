#include <vulkan/vulkan.hpp>

#include "../log.hpp"

namespace wg::internal::semaphoreUtil {

    void signalManySemaphoresFromManySemaphores(const std::vector<vk::Semaphore>& wait_semaphores,
                                                const std::vector<vk::Semaphore>& signal_semaphores,
                                                const vk::Queue& queue);

    void signalManySemaphoresFromSingleSemaphore(const std::vector<vk::Semaphore>& many_semaphores,
                                                 const vk::Semaphore& semaphore,
                                                 const vk::Queue& queue);

    void signalSemaphore(const vk::Semaphore& semaphore,
                         const vk::Queue& queue);

    void signalSemaphores(const std::vector<vk::Semaphore>& semaphores,
                          const vk::Queue& queue);
};
