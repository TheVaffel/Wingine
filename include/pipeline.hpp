
#ifndef WG_PIPELINE_HPP
#define WG_PIPELINE_HPP

#include "declarations.hpp"
#include "buffer.hpp"

namespace wg {

    struct Command {
        vk::CommandBuffer buffer;
        vk::Fence fence;
    };
  
    class Shader {
        vk::PipelineShaderStageCreateInfo shader_info;
    
        Shader(vk::Device& device,
               uint64_t stage,
               std::vector<uint32_t>& spirv);

        friend class Pipeline;
        friend class Wingine;
        friend class ComputePipeline;
    };
  
    // Future addition
    struct PipelineSetup {
        bool depthOnly = false;
        bool enableDepth = true;
        
        int width = -1;
        int height = -1;

        PipelineSetup& setDepthOnly(bool depthOnly);
        PipelineSetup& setEnableDepth(bool enableDepth);
        PipelineSetup& setWidth(int width);
        PipelineSetup& setHeight(int height);
    };
  
    class Pipeline {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        RenderPassType render_pass_type;
    
        Pipeline(Wingine& wing,
                 const std::vector<VertexAttribDesc>& descriptions,
                 const std::vector<ResourceSetLayout>& resourceSetLayout,
                 const std::vector<Shader*>& shaders,
                 const PipelineSetup& setup);

        friend class RenderFamily;
        friend class Wingine;
    };

    class ComputePipeline {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;

        Command command;

        ComputePipeline(Wingine& wing,
                        const std::vector<ResourceSetLayout>& resourceSetLayout,
                        Shader* shaders);

        friend class ComputeFamily;
        friend class Wingine;
    };
};

#endif // ndef WG_PIPELINE_HPP
