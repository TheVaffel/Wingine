#pragma once

#include <vulkan/vulkan.hpp>

namespace wg::internal {

    struct CommandLayoutTransitionData {
        vk::ImageMemoryBarrier image_memory_barrier;
    };

    struct CopyImageAuxillaryData {
        CommandLayoutTransitionData src_initial_transition_data,
            dst_initial_transition_data,
            src_final_transition_data,
            dst_final_transition_data;
    };
};
