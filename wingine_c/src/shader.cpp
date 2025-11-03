#include "./shader.h"

#include <vulkan/vulkan.h>
#include <Wingine.hpp>

#include "./wingine.h"
#include "./types.hpp"
#include "./catch.hpp"

EXTERNC

wg_shader_t* wg_create_shader(wg_wingine_t* wing, wg_shader_stage stage, const uint32_t* spv, uint32_t num_words) {
    const std::vector<uint32_t> spirv(spv, spv + num_words);

    VkShaderStageFlagBits shader_flag;
    switch (stage) {
    case WG_SHADER_STAGE_VERTEX:
        shader_flag = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case WG_SHADER_STAGE_FRAGMENT:
        shader_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case WG_SHADER_STAGE_COMPUTE:
        shader_flag = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    default:
	std::cerr << "[wingine_c::shader.cpp] Missing case for shader flag" << std::endl;
        throw std::runtime_error("[wingine_c::shader.cpp] Missing case for shader flag");
    }

    return new wg_shader_t {
        .v = catch_error(wing->wingine.createShader((wg::ShaderStage)shader_flag, spirv))
    };
}

void wg_destroy_shader(wg_shader_t* shader) {
    catch_error(delete shader);
}

EXTERNC_END
