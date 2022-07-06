#pragma once


#include "./IPipeline.hpp"
#include "./BasicPipelineSetup.hpp"
#include "./VertexAttribDesc.hpp"
#include "./IShader.hpp"

#include "../DeviceManager.hpp"
#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal {
    class BasicPipeline : public IPipeline {
        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;

        std::shared_ptr<DeviceManager> device_manager;

    public:

        BasicPipeline(const BasicPipelineSetup& setup,
                      const std::vector<VertexAttribDesc>& descriptions,
                      const std::vector<vk::DescriptorSetLayout>& resource_set_layout,
                      const std::vector<std::shared_ptr<IShader>>& shaders,
                      std::shared_ptr<DeviceManager> device_manager,
                      std::shared_ptr<CompatibleRenderPassRegistry> render_pass_registry,
                      const vk::PipelineCache& pipeline_cache);

        virtual vk::Pipeline getPipeline() const final;
        virtual vk::PipelineLayout getPipelineLayout() const final;

        ~BasicPipeline();
    };
};
