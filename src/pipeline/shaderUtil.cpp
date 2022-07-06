#include "./shaderUtil.hpp"

namespace wg::internal::shaderUtil {
    vk::ShaderStageFlagBits getShaderStageBit(ShaderStage stage) {
        return vk::ShaderStageFlagBits(stage);
    }
};
