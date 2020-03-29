#include "pipeline.hpp"

#include "Wingine.hpp"

namespace wg {

    
  Shader::Shader(vk::Device& device, uint64_t shader_type, std::vector<uint32_t>& spirv) {
    
    vk::ShaderStageFlagBits stage_bit;
    switch(shader_type) {
    case shaVertex:
      stage_bit = vk::ShaderStageFlagBits::eVertex;
      break;
    case shaFragment:
      stage_bit = vk::ShaderStageFlagBits::eFragment;
      break;
    }

    this->shader_info.setStage(stage_bit)
      .setPName("main");

    vk::ShaderModuleCreateInfo smci;
    smci.setCodeSize(spirv.size() * sizeof(uint32_t))
      .setPCode(spirv.data());
    
    
    this->shader_info.module = device.createShaderModule(smci);
    
  }
  
  
  Pipeline::Pipeline(Wingine& wing,
		     int width, int height,
		     const std::vector<VertexAttribDesc>& descriptions,
		     const std::vector<ResourceSetLayout>& resourceSetLayouts,
		     const std::vector<Shader*>& shaders, bool depthOnly) {

    vk::Device device = wing.getDevice();
    
    int vertex_binding_count = 0;
    
    for(VertexAttribDesc desc : descriptions) {
      vertex_binding_count = std::max(desc.binding_num + 1,
				      (uint32_t)vertex_binding_count);
    }
    

    // Defaults okay sometimes
    vk::PipelineDynamicStateCreateInfo state_info;
    std::vector<vk::VertexInputBindingDescription> vi_bindings(vertex_binding_count);
    std::vector<vk::VertexInputAttributeDescription> vi_attribs(descriptions.size());
    vk::PipelineVertexInputStateCreateInfo vi;
    vk::PipelineInputAssemblyStateCreateInfo ia;
    vk::PipelineRasterizationStateCreateInfo rs;
    vk::PipelineColorBlendStateCreateInfo cb;
    vk::PipelineColorBlendAttachmentState att_state;
    vk::PipelineViewportStateCreateInfo vp;
    vk::PipelineDepthStencilStateCreateInfo ds;
    vk::PipelineMultisampleStateCreateInfo ms;
    vk::PipelineLayoutCreateInfo layoutCreateInfo;

    vk::GraphicsPipelineCreateInfo createInfo;

    for(int i = 0; i < vertex_binding_count; i++) {
      vi_bindings[i].setBinding(i)
	.setInputRate(vk::VertexInputRate::eVertex);
    }

    for(unsigned int i = 0; i < descriptions.size(); i++) {
      vi_bindings[descriptions[i].binding_num]
	.setStride(descriptions[i].stride_in_bytes); // Set stride of binding structure

      vi_attribs[i].setBinding(descriptions[i].binding_num)
	.setLocation(i)
	.setFormat(_get_format(descriptions[i].component_type,
			       descriptions[i].num_elements))
	.setOffset(descriptions[i].offset_in_bytes);
    }
    
    vk::Viewport viewport;
    viewport.setWidth((float)width)
      .setHeight((float)height)
      .setMinDepth(0.0f)
      .setMaxDepth(1.0f)
      .setX(0.f)
      .setY(0.f);

    vk::Rect2D scissor;
    scissor.setExtent({(uint32_t)width, (uint32_t)height})
      .setOffset({(uint32_t)0, (uint32_t)0});
    
    vi.setVertexBindingDescriptionCount(vertex_binding_count)
      .setPVertexBindingDescriptions(vi_bindings.data())
      .setVertexAttributeDescriptionCount(descriptions.size())
      .setPVertexAttributeDescriptions(vi_attribs.data());

    ia.setPrimitiveRestartEnable(false)
      .setTopology(vk::PrimitiveTopology::eTriangleList);

    rs.setPolygonMode(vk::PolygonMode::eFill)
      .setCullMode(vk::CullModeFlagBits::eNone)
      .setFrontFace(vk::FrontFace::eClockwise)
      .setDepthClampEnable(false)
      .setRasterizerDiscardEnable(false)
      .setDepthBiasEnable(false)
      .setDepthBiasConstantFactor(0)
      .setDepthBiasClamp(0)
      .setDepthBiasSlopeFactor(0)
      .setLineWidth(1.0f);

    att_state.setColorWriteMask(vk::ColorComponentFlags(0xf))
      .setBlendEnable(true)
      .setAlphaBlendOp(vk::BlendOp::eAdd)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
      .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
      .setSrcAlphaBlendFactor(vk::BlendFactor::eZero)
      .setDstAlphaBlendFactor(vk::BlendFactor::eOne);
    
    cb.setPAttachments(&att_state)
      .setAttachmentCount(1)
      .setLogicOpEnable(false)
      .setLogicOp(vk::LogicOp::eNoOp)
      .setBlendConstants({1.0f, 1.0f,
	    1.0f, 1.0f});

    vp.setViewportCount(1)
      .setScissorCount(1)
      .setPScissors(&scissor)
      .setPViewports(&viewport);

    ds.setDepthTestEnable(true)
      .setDepthWriteEnable(true)
      .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
      .setDepthBoundsTestEnable(false)
      .setBack({vk::StencilOp::eKeep,
	    vk::StencilOp::eKeep,
	    vk::StencilOp::eKeep,
	    vk::CompareOp::eAlways,
	    0, 0, 0})
      .setMinDepthBounds(0)
      .setMaxDepthBounds(0)
      .setStencilTestEnable(false);
    ds.setFront(ds.back);

    ms.setPSampleMask(nullptr)
      .setRasterizationSamples(vk::SampleCountFlagBits::e1)
      .setSampleShadingEnable(false)
      .setAlphaToCoverageEnable(false)
      .setAlphaToOneEnable(false)
      .setMinSampleShading(0.0f);

    
    std::vector<vk::DescriptorSetLayout> layouts(resourceSetLayouts.size());
    for(unsigned int i = 0; i < resourceSetLayouts.size(); i++) {
      layouts[i] = resourceSetLayouts[i].layout;
    }
    
    layoutCreateInfo.setPushConstantRangeCount(0)
      .setPPushConstantRanges(nullptr)
      .setSetLayoutCount(layouts.size())
      .setPSetLayouts(layouts.data());

    this->layout = device.createPipelineLayout(layoutCreateInfo);

    std::vector<vk::PipelineShaderStageCreateInfo> pssci(shaders.size());
    for(unsigned int i = 0; i < shaders.size(); i++) {
      pssci[i] = shaders[i]->shader_info;
    }

    
    RenderPassType rpt;
      
    if (depthOnly) {
      rpt = renDepth;
    } else {
      rpt = renColorDepth;
    }

    

    if(wing.compatibleRenderPassMap.find(rpt) == wing.compatibleRenderPassMap.end()) {
      wing.register_compatible_render_pass(rpt);
    }
    
    
    createInfo.setLayout(this->layout)
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
      .setPStages(pssci.data())
      .setStageCount(pssci.size())
      .setRenderPass(wing.compatibleRenderPassMap[rpt])
      .setSubpass(0);

    this->render_pass_type = rpt;
    
    this->pipeline = device.createGraphicsPipelines(wing.pipeline_cache,
						    {createInfo})[0];    
  }

  
};
