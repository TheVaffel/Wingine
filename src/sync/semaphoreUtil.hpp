#include <vulkan/vulkan.hpp>

#include "../log.hpp"

namespace wg::internal::semaphoreUtil {

    void signalManySemaphoresFromSingleSemaphore(const std::vector<vk::Semaphore>& many_semaphores,
                                                 const vk::Semaphore& semaphore,
                                                 const vk::Queue& queue);

    void signalSemaphore(const vk::Semaphore& semaphore,
                         const vk::Queue& queue);
};
