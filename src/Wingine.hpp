#pragma once

#include <iostream>

#include <vector>
#include <map>
#include <limits>

#ifndef HEADLESS
#include <Winval.hpp>
#endif

#include "declarations.hpp"

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

#include "buffer.hpp"
#include "image.hpp"
#include "resource.hpp"
#include "compute_pipeline.hpp"
#include "semaphore.hpp"
#include "util.hpp"

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
        // std::shared_ptr<internal::StagingBufferManager> staging_buffer_manager;
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

        void cons_image_image(Image& image,
                              uint32_t width, uint32_t height,
                              vk::Format format,
                              vk::ImageUsageFlags usage,
                              vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                              vk::ImageLayout layout = vk::ImageLayout::eUndefined);

        void cons_image_memory(Image& image,
                               vk::MemoryPropertyFlags memProps);
        void cons_image_view(Image& image,
                             ImageViewType type,
                             vk::Format format);

        void copy_image(uint32_t w1, uint32_t h1, vk::Image src,
                        vk::ImageLayout srcCurrentLayout, vk::ImageLayout srcFinalLayout,
                        uint32_t w2, uint32_t h2, vk::Image dst,
                        vk::ImageLayout dstCurrentLayout, vk::ImageLayout dstFinalLayout,
                        vk::ImageAspectFlagBits flag,
                        const std::initializer_list<SemaphoreChain*>& wait_semaphores);

        void cmd_set_layout(const vk::CommandBuffer& commandBuffer, vk::Image image,
                            vk::ImageAspectFlagBits aspect, vk::ImageLayout currentLayout,
                            vk::ImageLayout finalLayout);

        // Don't delete color images, those are handled by swapchain
        void destroySwapchainImage(Image& image);

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

        void destroy(Image& image);

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

        ResourceSetChainPtr createResourceSetChain(const std::vector<uint64_t>& resourceLayout);

        StaticResourceChainPtr createStaticResourceChain(std::shared_ptr<internal::IResource> resource);

        ShaderPtr createShader(internal::ShaderStage shader_stage, const std::vector<uint32_t>& spirv);
        PipelinePtr createBasicPipeline(const std::vector<VertexAttribDesc>& descriptions,
                                        const std::vector<std::vector<uint64_t>>& resource_set_layout,
                                        const std::vector<ShaderPtr>& shaders,
                                        internal::BasicPipelineSetup setup = internal::BasicPipelineSetup());

        /* ComputePipeline* createComputePipeline(const std::vector<std::vector<uint64_t> >& resourceSetLayout,
           Shader* shaders); */
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

        ResourceImage* createResourceImage(uint32_t width, uint32_t height);

        TexturePtr createBasicTexture(uint32_t width, uint32_t height, const BasicTextureSetup& setup = BasicTextureSetup(internal::BasicImageSetup::createColorTextureImageSetup()));
        SettableTexturePtr createSettableTexture(uint32_t width, uint32_t height);
        StorageTexturePtr createStorageTexture(uint32_t width, uint32_t height);
        TextureChainPtr createBasicTextureChain(uint32_t width, uint32_t height, const BasicTextureSetup& setup = BasicTextureSetup(internal::BasicImageSetup::createColorTextureImageSetup()));

        SemaphoreChain* createSemaphoreChain();

        void dispatchCompute(ComputePipeline* compute,
                             const std::initializer_list<ResourceSet*>& resource_sets,
                             const std::initializer_list<SemaphoreChain*>& semaphores, int x_dim = 1, int y_dim = 1, int z_dim = 1);

        void setPresentWaitForSemaphores(internal::WaitSemaphoreSet&& semaphores);
        Semaphore createAndAddImageReadySemaphore();
        void setImageReadySemaphores(internal::SignalSemaphoreSet&& semaphores);

        EventChainPtr createEventChain();

        void present();

        uint32_t getRenderedImageRowByteStride() const;
        void copyLastRenderedImage(uint32_t* dst);

        void destroy(Buffer* buffer);
        void destroy(SemaphoreChain* semaphore_chain);
        void destroy(ResourceImage* resource_image);
        void destroy(ResourceSet* resource_set);
        void destroy(ComputePipeline* compute_pipeline);

#ifndef HEADLESS
        Wingine(Winval& win);

        Wingine(uint32_t width, uint32_t height,
                winval_type_0 arg0,
                winval_type_1 arg1,
                const std::string& app_name = "Wingine");
#endif

        Wingine(uint32_t width, uint32_t height, const std::string& app_name = "Wingine");

        ~Wingine();

        friend class Buffer;
        friend class RenderFamily;
        friend class ResourceSetLayout;
        friend class ResourceSet;
        friend class Texture;
        friend class SemaphoreChain;
        friend class Image;
        friend class ComputePipeline;
        friend class ResourceImage;
    };
};


#include "./Wingine.impl.hpp"
