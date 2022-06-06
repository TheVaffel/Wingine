
#ifndef WG_PIPELINE_HPP
#define WG_PIPELINE_HPP

#include "declarations.hpp"
#include "buffer.hpp"

#include "./CommandManager.hpp"
#include "./render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg {

    class Shader {
        vk::PipelineShaderStageCreateInfo shader_info;

        Shader(vk::Device& device,
               uint64_t stage,
               std::vector<uint32_t>& spirv);

        friend class Pipeline;
        friend class Wingine;
        friend class ComputePipeline;
    };

    enum class PolygonMode {
        Point,
        Line,
        Fill
    };

    struct PipelineSetup {
        bool depthOnly = false;
        bool enableDepth = true;

        int width = -1;
        int height = -1;

        PolygonMode polygonMode = PolygonMode::Fill;

        PipelineSetup& setDepthOnly(bool depthOnly);
        PipelineSetup& setEnableDepth(bool enableDepth);
        PipelineSetup& setWidth(int width);
        PipelineSetup& setHeight(int height);
        PipelineSetup& setPolygonMode(PolygonMode polygonMode);
    };

    class Pipeline {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        internal::renderPassUtil::RenderPassType render_pass_type;

        Pipeline(Wingine& wing,
                 const std::vector<VertexAttribDesc>& descriptions,
                 const std::vector<vk::DescriptorSetLayout>& resourceSetLayout,
                 const std::vector<Shader*>& shaders,
                 const PipelineSetup& setup);

    public:
        vk::Pipeline getPipeline() const;
        vk::PipelineLayout getLayout() const;

        friend class RenderFamily;
        friend class Wingine;
    };

    class ComputePipeline {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;

        internal::Command command;

        ComputePipeline(Wingine& wing,
                        const std::vector<vk::DescriptorSetLayout>& resourceSetLayout,
                        Shader* shaders);

        friend class ComputeFamily;
        friend class Wingine;
    };
};

#endif // ndef WG_PIPELINE_HPP
