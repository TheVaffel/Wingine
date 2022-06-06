#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace wg {
    enum ResourceType {
        resUniform = (uint64_t)VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        resStorageBuffer = (uint64_t)VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        resTexture = (uint64_t)VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        resImage = (uint64_t)VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    };

    // These are shifted, because we want to combine them
    // with resource types as a bitmask
    // Also, turns out we can't really assume compilers support 64-bit
    // enum values (looking at you, Windows), so we'll go the cheap way
    // and declare them as integers instead
    typedef int64_t ShaderType;
    const ShaderType shaVertex = (int64_t)VK_SHADER_STAGE_VERTEX_BIT << 32;
    const ShaderType shaFragment = (int64_t)VK_SHADER_STAGE_FRAGMENT_BIT << 32;
    const ShaderType shaCompute = (int64_t)VK_SHADER_STAGE_COMPUTE_BIT << 32;

};
