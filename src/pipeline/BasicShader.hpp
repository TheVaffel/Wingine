#pragma once


#include "../core/DeviceManager.hpp"

#include "./ShaderStage.hpp"
#include "./IShader.hpp"

namespace wg::internal {
    class BasicShader : public IShader {
        vk::PipelineShaderStageCreateInfo shader_info;
        vk::ShaderModule shader_module;

        std::vector<uint32_t> spirv_bytecode;

        std::shared_ptr<DeviceManager> device_manager;
    public:

        BasicShader(ShaderStage stage,
                    const std::vector<uint32_t>& spirv_bytecode,
                    std::shared_ptr<DeviceManager> device_manager);

        ~BasicShader();

        virtual vk::PipelineShaderStageCreateInfo getShaderInfo() const final;

    };
};
