#include "./BasicShader.hpp"

#include "./shaderUtil.hpp"

namespace wg::internal {
    BasicShader::BasicShader(ShaderStage stage,
                             const std::vector<uint32_t>& spirv_bytecode,
                             std::shared_ptr<DeviceManager> device_manager)
        : spirv_bytecode(spirv_bytecode), device_manager(device_manager) {

        vk::ShaderModuleCreateInfo module_info;
        module_info.setCodeSize(this->spirv_bytecode.size() * sizeof(uint32_t))
            .setPCode(spirv_bytecode.data());

        vk::ShaderStageFlagBits stage_bit = shaderUtil::getShaderStageBit(stage);

        this->shader_module = device_manager->getDevice().createShaderModule(module_info);

        this->shader_info
            .setStage(stage_bit)
            .setPName("main")
            .setModule(this->shader_module);
    }

    vk::PipelineShaderStageCreateInfo BasicShader::getShaderInfo() const {
        return this->shader_info;
    }

    BasicShader::~BasicShader() {
        this->device_manager->getDevice().destroy(this->shader_module);
    }
};
