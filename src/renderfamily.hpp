#ifndef WG_RENDERFAMILY_HPP
#define WG_RENDERFAMILY_HPP

#include "declarations.hpp"

#include "pipeline.hpp"
#include "resource.hpp"

#include "./framebuffer/IFramebufferChain.hpp"
#include "./render_pass/CompatibleRenderPassRegistry.hpp"

namespace wg {

    class RenderFamily {
        Wingine* wing;

        std::vector<internal::Command> commands;
        const Pipeline* pipeline;
        std::vector<vk::RenderPass> render_passes;
        bool clears;

        uint32_t num_buffers;

        std::shared_ptr<internal::IFramebufferChain> framebuffer_chain;
        std::shared_ptr<internal::CompatibleRenderPassRegistry> render_pass_registry;
        internal::renderPassUtil::RenderPassType render_pass_type;

        uint32_t current_framebuffer_index = 0;

        RenderFamily(Wingine& wing,
                     std::shared_ptr<internal::CompatibleRenderPassRegistry> renderPassRegistry,
                     const Pipeline* pipeline,
                     bool clear,
                     int num_buffers = 0);

        void submit_command(const std::initializer_list<SemaphoreChain*>& wait_semaphores, int index);

        void setFramebufferCount(uint32_t new_count);

    public:

        void startRecording(std::shared_ptr<internal::IFramebufferChain> framebuffer_chain);

        void recordDraw(const std::vector<const Buffer*>& buffers, const IndexBuffer* ind_buf,
                        const std::vector<ResourceSet*>& sets, int instanceCount = 1);
        void endRecording();

        void submit(const std::initializer_list<SemaphoreChain*>& wait_semaphores = {}, int index = -1);

        friend class Wingine;
    };

    class ComputeFamily {
        Wingine* wing;

        ComputeFamily(Wingine& wing,
                      Pipeline* pipeline);

    public:

        void startRecording();
        void recordCompute(const std::vector<ResourceSet*>& set, int x_extent, int y_extent, int z_extent);
        void endRecording();

        void submit(const std::initializer_list<SemaphoreChain*>& wait_semaphores = {}, int index = -1);

        friend class Wingine;
    };
};
#endif // WG_RENDERFAMILY_HPP
