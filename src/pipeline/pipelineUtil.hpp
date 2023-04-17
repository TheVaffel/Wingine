#pragma once

#include "./ComponentType.hpp"
#include "./VertexAttribDesc.hpp"
#include "./IShader.hpp"
#include "./types.hpp"
#include "./WrappedPLCI.hpp"

#include "../render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg::internal::pipelineUtil {
    vk::Format getFormat(ComponentType type,
                         uint32_t num_components);

    vk::PipelineDynamicStateCreateInfo getDefaultDynamicInfo();

    vk::PolygonMode getPolygonMode(PolygonMode mode);
    uint32_t getBindingCount(const std::vector<VertexAttribDesc>& vertexAttribs);

    std::vector<vk::VertexInputBindingDescription>
    createInputBindings(uint32_t count, const std::vector<VertexAttribDesc>& descriptions);

    std::vector<vk::VertexInputAttributeDescription>
    createInputAttributes(const std::vector<VertexAttribDesc>& descriptions);

    std::vector<vk::VertexInputAttributeDescription> createInputAttributes();

    vk::PipelineVertexInputStateCreateInfo createVertexInputInfo(
        const std::vector<vk::VertexInputBindingDescription>& input_bindings,
        const std::vector<vk::VertexInputAttributeDescription>& input_attributes);

    vk::PipelineInputAssemblyStateCreateInfo getDefaultInputAssemblyInfo();
    vk::PipelineRasterizationStateCreateInfo createRasterizationInfo(vk::PolygonMode polygonMode);

    vk::PipelineColorBlendStateCreateInfo getDefaultColorBlendInfo(vk::PipelineColorBlendAttachmentState& att_state);
    vk::PipelineViewportStateCreateInfo createViewportInfo(uint32_t width,
                                                           uint32_t height,
                                                           vk::Viewport& viewport,
                                                           vk::Rect2D& scissor);

    vk::PipelineDepthStencilStateCreateInfo createDepthStencilInfo(bool depthEnabled);
    vk::PipelineMultisampleStateCreateInfo getDefaultMultisampleInfo();
    WrappedPLCI createLayoutInfo(const std::map<uint32_t, vk::DescriptorSetLayout>& resourceSetLayouts);

    std::vector<vk::PipelineShaderStageCreateInfo> getShaderInfo(std::vector<std::shared_ptr<IShader>> shaders);

    vk::RenderPass getCompatibleRenderPass(bool depthOnly,
                                           std::shared_ptr<CompatibleRenderPassRegistry> render_pass_registry);

#ifdef DEBUG
        void checkAttributeConsistency(uint32_t binding_count,
                                       const std::vector<VertexAttribDesc>& attrib_descriptions);
#endif // DEBUG
};
