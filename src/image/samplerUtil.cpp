#include "./samplerUtil.hpp"

namespace wg::internal::samplerUtil {

    vk::Sampler createBasicSampler(const BasicSamplerSetup& setup,
                                   const vk::Device& device) {
        vk::SamplerCreateInfo info;

        info.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
            .setAddressModeV(vk::SamplerAddressMode::eClampToBorder);
        return device.createSampler(info);
    }
};
