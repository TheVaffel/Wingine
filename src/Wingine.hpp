#pragma once

#include <iostream>

#include <vector>
#include <map>
#include <limits>

#ifndef HEADLESS
#include <Winval.hpp>
#endif

#include "./render_pass/CompatibleRenderPassRegistry.hpp"
#include "./VulkanInstanceManager.hpp"
#include "./DeviceManager.hpp"
#include "./QueueManager.hpp"
#include "./image/HostVisibleImageView.hpp"
#include "./draw_pass/BasicDrawPassSettings.hpp"
#include "./framebuffer/SwapchainFramebufferChain.hpp"
#include "./resource/ResourceSetLayoutRegistry.hpp"

#include "./pipeline/BasicPipelineSetup.hpp"
#include "./pipeline/ShaderStage.hpp"

#include "./types.hpp"

namespace wg {


    class Wingine {

        std::shared_ptr<internal::VulkanInstanceManager> vulkan_instance_manager;

        std::shared_ptr<internal::DeviceManager> device_manager;

        // This one will be used a lot (at least currently),
        // so will cache from device_manager, if for no other reason to shorten code
        vk::Device device;

        std::shared_ptr<internal::QueueManager> queue_manager;
        std::shared_ptr<internal::CommandManager> command_manager;
        std::shared_ptr<internal::IFramebufferChain> default_framebuffer_chain;

        std::shared_ptr<internal::HostVisibleImageView> host_visible_image;
        std::shared_ptr<internal::IImage> host_accessible_image;

        std::shared_ptr<internal::ResourceSetLayoutRegistry> resource_set_layout_registry;

        // General purpose fence for the moving phase
        vk::Fence general_purpose_fence;

        vk::DescriptorPool descriptor_pool;

        vk::PipelineCache pipeline_cache;

        uint32_t window_width, window_height;

        std::shared_ptr<internal::CompatibleRenderPassRegistry> compatible_render_pass_registry;

        class VulkanInitInfo {
            vk::Extent2D dimensions;
            std::string application_name;
            bool is_headless = false;

#ifndef HEADLESS

            winval_type_0 win_arg0;
            winval_type_1 win_arg1;
#endif
        public:

            const vk::Extent2D& getDimensions() const;
            std::string getApplicationName() const;
            bool getIsHeadless() const;

#ifndef HEADLESS
            winval_type_0 getWinArg0() const;
            winval_type_1 getWinArg1() const;
#endif

            VulkanInitInfo(uint32_t width, uint32_t height, const std::string& application_name);

#ifndef HEADLESS
            VulkanInitInfo(uint32_t width,
                           uint32_t height,
                           winval_type_0 arg0,
                           winval_type_1 arg1,
                           const std::string& application_name);
#endif
        };


        void init_vulkan(const VulkanInitInfo& init_info);

        void init_generic_render_pass();
        void init_descriptor_pool();
        void init_pipeline_cache();

        vk::Queue getGraphicsQueue();
        vk::Queue getPresentQueue();
        vk::Device getDevice();
        const internal::Command& getGeneralCommand();
        vk::DescriptorPool getDescriptorPool();
        vk::CommandPool getPresentCommandPool();
        vk::CommandPool getGraphicsCommandPool();

        void register_compatible_render_pass(internal::renderPassUtil::RenderPassType type);
        vk::RenderPass create_render_pass(internal::renderPassUtil::RenderPassType type,
                                          bool clear);


    public:

        void waitIdle();

        const internal::IFramebuffer& getCurrentFramebuffer();
        int getNumFramebuffers();

        int getWindowWidth();
        int getWindowHeight();

        template<typename T>
        VertexBufferPtr<T> createVertexBuffer(uint32_t element_count);

        template <typename T>
        StorageBufferPtr<T> createStorageBuffer(uint32_t element_count);

        IndexBufferPtr createIndexBuffer(uint32_t num_indices);

        template<typename T>
        UniformPtr<T> createUniform();

        template<typename T>
        UniformChainPtr<T> createUniformChain();

        DrawPassPtr createBasicDrawPass(PipelinePtr pipeline, const internal::BasicDrawPassSettings& settings);

        template<typename... Ts>
        ResourceSetChainPtr createResourceSetChain(const std::vector<uint64_t>& resourceLayout,
                                                   Ts... resources);

        ShaderPtr createShader(internal::ShaderStage shader_stage, const std::vector<uint32_t>& spirv);
        PipelinePtr createBasicPipeline(const std::vector<VertexAttribDesc>& descriptions,
                                        const std::vector<std::vector<uint64_t>>& resource_set_layout,
                                        const std::vector<ShaderPtr>& shaders,
                                        internal::BasicPipelineSetup setup = internal::BasicPipelineSetup());

        ComputePipelinePtr createComputePipeline(const std::vector<std::vector<uint64_t>>& resource_set_layout,
                                                 ShaderPtr shader);

        Framebuffer createFramebuffer(uint32_t width, uint32_t height,
                                      bool depthOnly = false);

        FramebufferChain createFramebufferChain(uint32_t width,
                                                uint32_t height,
                                                bool depthOnly = false,
                                                uint32_t count = 3);

        FramebufferTextureChainPtr createFramebufferTextureChain(uint32_t width,
                                                                 uint32_t height,
                                                                 bool depth_only = false,
                                                                 uint32_t count = std::numeric_limits<uint32_t>::max());

        ImageCopierPtr createImageCopier();
        ImageChainCopierPtr createImageChainCopier();

        FramebufferChain getDefaultFramebufferChain();

        TexturePtr createBasicTexture(uint32_t width, uint32_t height, const BasicTextureSetup& setup = BasicTextureSetup(internal::BasicImageSetup::createColorTextureImageSetup()));
        SettableTexturePtr createSettableTexture(uint32_t width, uint32_t height);
        StorageTexturePtr createStorageTexture(uint32_t width, uint32_t height);
        TextureChainPtr createBasicTextureChain(uint32_t width, uint32_t height, const BasicTextureSetup& setup = BasicTextureSetup(internal::BasicImageSetup::createColorTextureImageSetup()));

        void setPresentWaitForSemaphores(internal::WaitSemaphoreSet&& semaphores);
        Semaphore createAndAddImageReadySemaphore();
        void setImageReadySemaphores(internal::SignalSemaphoreSet&& semaphores);

        EventChainPtr createEventChain();

        void present();

        uint32_t getRenderedImageRowByteStride() const;
        void copyLastRenderedImage(uint32_t* dst);

#ifndef HEADLESS
        Wingine(Winval& win);

        Wingine(uint32_t width, uint32_t height,
                winval_type_0 arg0,
                winval_type_1 arg1,
                const std::string& app_name = "Wingine");
#endif

        Wingine(uint32_t width, uint32_t height, const std::string& app_name = "Wingine");

        ~Wingine();
    };
};


#include "./Wingine.impl.hpp"
