#pragma once

#include "./IImage.hpp"
#include "../buffer/IBuffer.hpp"
#include "../CommandManager.hpp"
#include "./CopyImageAuxillaryData.hpp"

namespace wg::internal::copyImage {
    void recordCopyImage(IImage& src,
                         IImage& dst,
                         CopyImageAuxillaryData& auxillary_data,
                         const Command& command,
                         const vk::Device& device,
                         const vk::Queue& queue);

    void recordCopyImageToBuffer(IImage& src,
                                 IBuffer& dst,
                                 CopyImageToBufferAuxillaryData& auxillary_data,
                                 const Command& command,
                                 const vk::Device& device,
                                 const vk::Queue& queue);

};
