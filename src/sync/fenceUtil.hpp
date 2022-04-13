#include <vulkan/vulkan.hpp>

namespace wg::internal::fenceUtil {

    void awaitFence(const vk::Fence& fence,
                    const vk::Device& device);

    void resetFence(const vk::Fence& fence,
                    const vk::Device& device);

    void awaitAndResetFence(const vk::Fence& fence,
                            const vk::Device& device);
};
