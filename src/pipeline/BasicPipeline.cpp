#include "./BasicPipeline.hpp"

#include "./pipelineUtil.hpp"
#include "../render_pass/renderPassUtil.hpp"
#include "../spirv/util.hpp"

#include <iostream>

#include "../resource/WrappedDSLCI.hpp"
#include "../resource/descriptorUtil.hpp"

namespace wg::internal {

    BasicPipeline::BasicPipeline(const BasicPipelineSetup& setup,
                                 const std::vector<VertexAttribDesc>& attribute_descriptions,
                                 const std::vector<std::shared_ptr<IShader>>& shaders,
                                 std::shared_ptr<DeviceManager> device_manager,
                                 std::shared_ptr<CompatibleRenderPassRegistry> render_pass_registry,
                                 const vk::PipelineCache& pipeline_cache)
    : device_manager(device_manager) {

        vk::Device device = device_manager->getDevice();

        uint32_t width = setup.width;
        uint32_t height = setup.height;
        uint32_t vertex_binding_count = pipelineUtil::getBindingCount(attribute_descriptions);

#ifdef DEBUG
        pipelineUtil::checkAttributeConsistency(vertex_binding_count,
                                                attribute_descriptions);
#endif // DEBUG

        vk::PolygonMode polygon_mode = pipelineUtil::getPolygonMode(setup.polygonMode);

        std::vector<vk::VertexInputBindingDescription> vi_bindings =
            pipelineUtil::createInputBindings(vertex_binding_count, attribute_descriptions);
        std::vector<vk::VertexInputAttributeDescription> vi_attribs =
            pipelineUtil::createInputAttributes(attribute_descriptions);

        vk::PipelineDynamicStateCreateInfo state_info = pipelineUtil::getDefaultDynamicInfo();

        vk::PipelineVertexInputStateCreateInfo vi =
            pipelineUtil::createVertexInputInfo(vi_bindings, vi_attribs);

        vk::PipelineInputAssemblyStateCreateInfo ia = pipelineUtil::getDefaultInputAssemblyInfo();
        vk::PipelineRasterizationStateCreateInfo rs = pipelineUtil::createRasterizationInfo(polygon_mode);

        vk::PipelineColorBlendAttachmentState att_state;
        vk::PipelineColorBlendStateCreateInfo cb = pipelineUtil::getDefaultColorBlendInfo(att_state);

        vk::Viewport viewport;
        vk::Rect2D scissor;
        vk::PipelineViewportStateCreateInfo vp = pipelineUtil::createViewportInfo(width, height, viewport, scissor);
        vk::PipelineDepthStencilStateCreateInfo ds = pipelineUtil::createDepthStencilInfo(setup.enableDepth);
        vk::PipelineMultisampleStateCreateInfo ms = pipelineUtil::getDefaultMultisampleInfo();

        std::vector<std::span<const spirv::DescriptorSetLayout>> layouts;

        for (auto& shader : shaders) {
            layouts.push_back(shader->getLayouts());
        }

        std::map<uint32_t, WrappedDSLCI> create_infos = spirv::util::getDescriptorSetCreateInfos(layouts);
        std::map<uint32_t, vk::DescriptorSetLayout> resource_set_layouts =
            descriptorUtil::createDescriptorSetLayoutFromInfos(create_infos,
                                                            this->device_manager->getDevice());
        WrappedPLCI layoutCreateInfo = pipelineUtil::createLayoutInfo(resource_set_layouts);

        std::vector<vk::PipelineShaderStageCreateInfo> pssci = pipelineUtil::getShaderInfo(shaders);

        vk::RenderPass compatible_render_pass = pipelineUtil::getCompatibleRenderPass(setup.depthOnly,
                                                                                      render_pass_registry);

        this->layout_info.layout = device.createPipelineLayout(layoutCreateInfo.getCreateInfo());
        this->layout_info.set_layout_info_map = create_infos;
        this->layout_info.set_layout_map = resource_set_layouts;

        vk::GraphicsPipelineCreateInfo createInfo;
        createInfo.setLayout(this->layout_info.layout)
            .setBasePipelineHandle(nullptr)
            .setBasePipelineIndex(0)
            .setPVertexInputState(&vi)
            .setPInputAssemblyState(&ia)
            .setPRasterizationState(&rs)
            .setPColorBlendState(&cb)
            .setPTessellationState(nullptr)
            .setPMultisampleState(&ms)
            .setPDynamicState(&state_info)
            .setPViewportState(&vp)
            .setPDepthStencilState(&ds)
            .setStages(pssci)
            .setRenderPass(compatible_render_pass)
            .setSubpass(0);

        this->pipeline = device.createGraphicsPipelines(pipeline_cache,
                                                        {createInfo}).value[0];
    }

    vk::Pipeline BasicPipeline::getPipeline() const {
        return this->pipeline;
    }

    const PipelineLayoutInfo& BasicPipeline::getPipelineInfo() const {
        return this->layout_info;
    }

    BasicPipeline::~BasicPipeline() {
        this->device_manager->getDevice().destroy(this->pipeline);
        this->device_manager->getDevice().destroy(this->layout_info.layout);
        for (auto& layout : this->layout_info.set_layout_map) {
            this->device_manager->getDevice().destroy(layout.second);
        }
    }
};
