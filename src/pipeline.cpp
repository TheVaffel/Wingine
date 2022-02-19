#include "./pipeline.hpp"

#include "./Wingine.hpp"

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
        case shaCompute:
            stage_bit = vk::ShaderStageFlagBits::eCompute;
            break;
        default:
            std::cout << "[Wingine::Shader] Shader stage not yet supported" << std::endl;
        }

        this->shader_info.setStage(stage_bit)
            .setPName("main");

        vk::ShaderModuleCreateInfo smci;
        smci.setCodeSize(spirv.size() * sizeof(uint32_t))
            .setPCode(spirv.data());


        this->shader_info.module = device.createShaderModule(smci);

    }

    ComputePipeline::ComputePipeline(Wingine& wing,
                                     const std::vector<ResourceSetLayout>& resourceSetLayouts,
                                     Shader* shader) {
        vk::Device device = wing.getDevice();

        std::vector<vk::DescriptorSetLayout> layouts(resourceSetLayouts.size());
        for(unsigned int i = 0; i < resourceSetLayouts.size(); i++) {
            layouts[i] = resourceSetLayouts[i].layout;
        }

        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr)
            .setSetLayoutCount(layouts.size())
            .setPSetLayouts(layouts.data());

        this->layout = device.createPipelineLayout(layoutCreateInfo);

        vk::ComputePipelineCreateInfo cpci;
        cpci.setLayout(this->layout)
            .setStage(shader->shader_info);

        this->pipeline = device.createComputePipeline(wing.pipeline_cache,
                                                      {cpci}).value;

        vk::CommandBufferAllocateInfo cbi;
        cbi.setCommandPool(wing.getGraphicsCommandPool())
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
        command.buffer = device.allocateCommandBuffers(cbi)[0];

        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

        command.fence = device.createFence(fci);
    }

    PipelineSetup& PipelineSetup::setDepthOnly(bool depthOnly) {
        this->depthOnly = depthOnly;
        return *this;
    }

    PipelineSetup& PipelineSetup::setEnableDepth(bool enableDepth) {
        this->enableDepth = enableDepth;
        return *this;
    }

    PipelineSetup& PipelineSetup::setWidth(int width) {
        this->width = width;
        return *this;
    }

    PipelineSetup& PipelineSetup::setHeight(int height) {
        this->height = height;
        return *this;
    }

    PipelineSetup& PipelineSetup::setPolygonMode(PolygonMode polygonMode) {
        this->polygonMode = polygonMode;
        return *this;
    }

    Pipeline::Pipeline(Wingine& wing,
                       const std::vector<VertexAttribDesc>& descriptions,
                       const std::vector<ResourceSetLayout>& resourceSetLayouts,
                       const std::vector<Shader*>& shaders,
                       const PipelineSetup& setup) {

        vk::Device device = wing.getDevice();

        int width = setup.width == -1 ? wing.getWindowWidth() : setup.width;
        int height = setup.height == -1 ? wing.getWindowHeight() : setup.height;

        int vertex_binding_count = 0;

        vk::PolygonMode vkPolygonMode;
        switch (setup.polygonMode) {
        case PolygonMode::Point: vkPolygonMode = vk::PolygonMode::ePoint; break;
        case PolygonMode::Line: vkPolygonMode = vk::PolygonMode::eLine; break;
        case PolygonMode::Fill: vkPolygonMode = vk::PolygonMode::eFill; break;
        }


        for(const VertexAttribDesc& desc : descriptions) {
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

#ifdef DEBUG

        // Check consistency of input parameters
        std::vector<int> per_instance(vertex_binding_count);
        std::vector<int> stride(vertex_binding_count);

        for(int i = 0; i < vertex_binding_count; i++) {
            per_instance[i] = -1;
            stride[i] = -1;
        }
        for(unsigned int i = 0; i < descriptions.size(); i++) {
            if(per_instance[i] == -1) {
                per_instance[i] = descriptions[i].per_instance ? 1 : 0;
                stride[i] = descriptions[i].stride_in_bytes;
            } else {
                if(per_instance[i] != (descriptions[i].per_instance ? 1 : 0)) {
                    std::cerr << "[Wingine] Vertex atrribute descriptions' per-instance fields not consistent on same binding"
                              << std::endl;
                    std::exit(-1);
                }
                if(stride[i] != (int)(descriptions[i].stride_in_bytes)) {
                    std::cerr << "[Wingine] Vertex attribute descriptions' stride fields not consistent on same binding"
                              << std::endl;
                    std::exit(-1);
                }
            }
        }

#endif // DEBUG

        for(int i = 0; i < vertex_binding_count; i++) {
            vi_bindings[i].setBinding(i)
                .setInputRate(vk::VertexInputRate::eVertex);
        }

        for(unsigned int i = 0; i < descriptions.size(); i++) {
            vi_bindings[descriptions[i].binding_num]
                .setInputRate(descriptions[i].per_instance ?
                              vk::VertexInputRate::eInstance :
                              vk::VertexInputRate::eVertex)
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

        rs.setPolygonMode(vkPolygonMode)
            .setCullMode(vk::CullModeFlagBits::eBack)
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

        ds.setDepthTestEnable(setup.enableDepth)
            .setDepthWriteEnable(setup.enableDepth)
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

        if (setup.depthOnly) {
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
                                                        {createInfo}).value[0];
    }


};
