#pragma once

#include "./IImage.hpp"
#include "../CommandManager.hpp"
#include "./CopyImageAuxillaryData.hpp"

namespace wg::internal::copyImage {
    void recordCopyImage(IImage& src,
                         IImage& dst,
                         CopyImageAuxillaryData& auxillary_data,
                         const Command& command,
                         const vk::Device& device,
                         const vk::Queue& queue);
};
