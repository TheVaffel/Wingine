#include "./pipelineUtil.hpp"

#include <flawed_assert.hpp>

#include "../render_pass/renderPassUtil.hpp"

namespace wg::internal::pipelineUtil {

    vk::Format getFormat(ComponentType type,
                         uint32_t num_components) {
        fl_assert_gt(num_components, 0u);
        fl_assert_le(num_components, 4u);

        // VK_FORMAT_R32_SFLOAT = 100 etc.. are defined in vulkan_core.h

        switch(type) {
        case ComponentType::Float32:
            return vk::Format(97 + num_components * 3);
        case ComponentType::Float64:
            return vk::Format(109 + num_components * 3);
        case ComponentType::Int32:
            return vk::Format(96 + num_components * 3);
        case ComponentType::Int64:
            return vk::Format(95 + num_components * 3);
        }
        throw std::runtime_error("[pipelineUtil] Unrecognized component type + format combination");
    }

    vk::PolygonMode getPolygonMode(PolygonMode mode) {
        switch (mode) {
        case PolygonMode::Point: return vk::PolygonMode::ePoint;
        case PolygonMode::Line: return vk::PolygonMode::eLine;
        case PolygonMode::Fill: return vk::PolygonMode::eFill;
        }
        throw std::runtime_error("[pipelineUtil] Unrecognized polygon mode");
    }

    uint32_t getBindingCount(const std::vector<VertexAttribDesc>& vertex_attribs) {
        uint32_t count = 0;
        for (const VertexAttribDesc& desc : vertex_attribs) {
            count = std::max(desc.binding_num + 1, count);
        }

        return count;
    }

    vk::PipelineDynamicStateCreateInfo getDefaultDynamicInfo() {
        return vk::PipelineDynamicStateCreateInfo();
    }

    std::vector<vk::VertexInputBindingDescription>
    createInputBindings(uint32_t count, const std::vector<VertexAttribDesc>& descriptions) {
        std::vector<vk::VertexInputBindingDescription> bindings(count);

        // Initialize all bindings (including possibly unused ones)
        for(uint32_t i = 0; i < count; i++) {
            bindings[i].setBinding(i)
                .setInputRate(vk::VertexInputRate::eVertex);
        }

        // Further initialize used bindings
        for(unsigned int i = 0; i < descriptions.size(); i++) {
            bindings[descriptions[i].binding_num]
                .setInputRate(descriptions[i].per_instance ?
                              vk::VertexInputRate::eInstance :
                              vk::VertexInputRate::eVertex)
                .setStride(descriptions[i].stride_in_bytes); // Set stride of binding structure
        }

        return bindings;
    }

    std::vector<vk::VertexInputAttributeDescription>
    createInputAttributes(const std::vector<VertexAttribDesc>& descriptions) {
        std::vector<vk::VertexInputAttributeDescription> attribs(descriptions.size());

        for(unsigned int i = 0; i < descriptions.size(); i++) {
            attribs[i].setBinding(descriptions[i].binding_num)
                .setLocation(i)
                .setFormat(pipelineUtil::getFormat(descriptions[i].component_type,
                                                   descriptions[i].num_elements))
                .setOffset(descriptions[i].offset_in_bytes);
        }

        return attribs;
    }


    vk::PipelineVertexInputStateCreateInfo createVertexInputInfo(
        const std::vector<vk::VertexInputBindingDescription>& input_bindings,
        const std::vector<vk::VertexInputAttributeDescription>& input_attributes) {

        vk::PipelineVertexInputStateCreateInfo vi;
        vi.setVertexBindingDescriptionCount(input_bindings.size())
            .setPVertexBindingDescriptions(input_bindings.data())
            .setVertexAttributeDescriptionCount(input_attributes.size())
            .setPVertexAttributeDescriptions(input_attributes.data());

        return vi;
    }

    vk::PipelineInputAssemblyStateCreateInfo getDefaultInputAssemblyInfo() {
        vk::PipelineInputAssemblyStateCreateInfo ia;
        ia.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
        return ia;
    }

    vk::PipelineRasterizationStateCreateInfo createRasterizationInfo(vk::PolygonMode polygon_mode) {
        vk::PipelineRasterizationStateCreateInfo rs;

        rs.setPolygonMode(polygon_mode)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setDepthClampEnable(false)
            .setRasterizerDiscardEnable(false)
            .setDepthBiasEnable(false)
            .setDepthBiasConstantFactor(0)
            .setDepthBiasClamp(0)
            .setDepthBiasSlopeFactor(0)
            .setLineWidth(1.0f);

        return rs;
    }

    vk::PipelineColorBlendStateCreateInfo
    getDefaultColorBlendInfo(vk::PipelineColorBlendAttachmentState& att_state) {
        vk::PipelineColorBlendStateCreateInfo cb;

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

        return cb;
    }

    vk::PipelineViewportStateCreateInfo createViewportInfo(uint32_t width,
                                                           uint32_t height,
                                                           vk::Viewport& viewport,
                                                           vk::Rect2D& scissor) {
        viewport.setWidth((float)width)
            .setHeight((float)height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f)
            .setX(0.f)
            .setY(0.f);
        scissor.setExtent({(uint32_t)width, (uint32_t)height})
            .setOffset({(uint32_t)0, (uint32_t)0});

        vk::PipelineViewportStateCreateInfo vp;
        vp.setViewportCount(1)
            .setScissorCount(1)
            .setPScissors(&scissor)
            .setPViewports(&viewport);

        return vp;
    }


    vk::PipelineDepthStencilStateCreateInfo createDepthStencilInfo(bool depthEnabled) {

        vk::PipelineDepthStencilStateCreateInfo ds;
        ds.setDepthTestEnable(depthEnabled)
            .setDepthWriteEnable(depthEnabled)
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

        return ds;
    }

    vk::PipelineMultisampleStateCreateInfo getDefaultMultisampleInfo() {
        vk::PipelineMultisampleStateCreateInfo ms;
        ms.setPSampleMask(nullptr)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(false)
            .setAlphaToCoverageEnable(false)
            .setAlphaToOneEnable(false)
            .setMinSampleShading(0.0f);

        return ms;
    }

    vk::PipelineLayoutCreateInfo createLayoutInfo(const std::vector<vk::DescriptorSetLayout>& resourceSetLayouts) {

        vk::PipelineLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setPushConstantRangeCount(0)
            .setPPushConstantRanges(nullptr)
            .setSetLayouts(resourceSetLayouts);

        return layoutCreateInfo;
    }

    std::vector<vk::PipelineShaderStageCreateInfo> getShaderInfo(std::vector<std::shared_ptr<IShader>> shaders) {
        std::vector<vk::PipelineShaderStageCreateInfo> pssci(shaders.size());
        for(unsigned int i = 0; i < shaders.size(); i++) {
            pssci[i] = shaders[i]->getShaderInfo();
        }
        return pssci;
    }

    vk::RenderPass getCompatibleRenderPass(bool depthOnly,
                                           std::shared_ptr<CompatibleRenderPassRegistry> render_pass_registry) {

        renderPassUtil::RenderPassType rpt;

        if (depthOnly) {
            rpt = internal::renderPassUtil::RenderPassType::depthOnly;
        } else {
            rpt = internal::renderPassUtil::RenderPassType::colorDepth;
        }

        return render_pass_registry->ensureAndGetRenderPass(rpt);
    }

#ifdef DEBUG
        void checkAttributeConsistency(uint32_t binding_count,
                                       const std::vector<VertexAttribDesc>& attrib_descriptions) {

            // Check consistency of input parameters
            std::vector<int> per_instance = std::vector<int>(binding_count);
            std::vector<int> stride = std::vector<int>(binding_count);

            for(uint32_t i = 0; i < binding_count; i++) {
                per_instance[i] = -1;
                stride[i] = -1;
            }
            for(unsigned int i = 0; i < attrib_descriptions.size(); i++) {
                if(per_instance[i] == -1) {
                    per_instance[i] = attrib_descriptions[i].per_instance ? 1 : 0;
                    stride[i] = attrib_descriptions[i].stride_in_bytes;
                } else {
                    if(per_instance[i] != (attrib_descriptions[i].per_instance ? 1 : 0)) {
                        std::cerr << "[Wingine] Vertex atrribute descriptions' per-instance fields not consistent on same binding"
                                  << std::endl;
                        std::exit(-1);
                    }
                    if(stride[i] != (int)(attrib_descriptions[i].stride_in_bytes)) {
                        std::cerr << "[Wingine] Vertex attribute descriptions' stride fields not consistent on same binding"
                                  << std::endl;
                        std::exit(-1);
                    }
                }
            }
        }
#endif // DEBUG
};
