#pragma once

#include <iostream>

#include <vector>
#include <map>

#include <Winval.hpp>

#include "declarations.hpp"

#include "./render_pass/CompatibleRenderPassRegistry.hpp"
#include "./VulkanInstanceManager.hpp"
#include "./DeviceManager.hpp"
#include "./QueueManager.hpp"
#include "./DefaultFramebufferManager.hpp"
#include "./framebuffer/IFramebuffer.hpp"
#include "./draw_pass/BasicDrawPassSettings.hpp"

#include "buffer.hpp"
#include "image.hpp"
#include "resource.hpp"
#include "pipeline.hpp"
#include "renderfamily.hpp"
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
        std::shared_ptr<internal::DefaultFramebufferManager> default_framebuffer_manager;

        // General purpose fence for the moving phase
        vk::Fence general_purpose_fence;

        vk::DescriptorPool descriptor_pool;

        vk::PipelineCache pipeline_cache;

        uint32_t window_width, window_height;

        std::map<std::vector<uint64_t>, ResourceSetLayout> resourceSetLayoutMap;

        std::shared_ptr<internal::CompatibleRenderPassRegistry> compatibleRenderPassRegistry;

        void init_vulkan(int width, int height,
                         winval_type_0 arg0,
                         winval_type_1 arg1,
                         const std::string& application_name);

        void init_vulkan(int width, int height, const std::string& app_name);

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

        void ensure_resource_set_layout_exists(const std::vector<uint64_t>& resourceSetLayout);

        // Don't delete color images, those are handled by swapchain
        void destroySwapchainImage(Image& image);

        vk::Queue getGraphicsQueue();
        vk::Queue getPresentQueue();
        vk::Device getDevice();
        const internal::Command& getGeneralCommand();
        vk::DescriptorPool getDescriptorPool();
        vk::CommandPool getPresentCommandPool();
        vk::CommandPool getGraphicsCommandPool();

        const std::vector<std::unique_ptr<internal::IFramebuffer>>& getFramebuffers();

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

        template<typename Type>
        VertexBuffer<Type>* createVertexBuffer(uint32_t num, bool host_updatable = true);

        IndexBuffer* createIndexBuffer(uint32_t num_indices);

        template<typename Type>
        Uniform<Type>* createUniform();

        StorageBuffer* createStorageBuffer(uint32_t num_bytes, bool host_updatable = true);

        RenderFamily* createRenderFamily(const Pipeline* pipeline, bool clear, int num_framebuffers = 0);
        DrawPassPtr createBasicDrawPass(const Pipeline* pipeline, const internal::BasicDrawPassSettings& settings);

        ResourceSet* createResourceSet(const std::vector<uint64_t>& resourceLayout);

        Shader* createShader(uint64_t stage_bit, std::vector<uint32_t>& spirv);

        Pipeline* createPipeline(const std::vector<VertexAttribDesc>& descriptions,
                                 const std::vector<std::vector<uint64_t> >& resourceSetLayout,
                                 const std::vector<Shader*>& shaders,
                                 const PipelineSetup& setup = PipelineSetup());

        ComputePipeline* createComputePipeline(const std::vector<std::vector<uint64_t> >& resourceSetLayout,
                                               Shader* shaders);

        Framebuffer createFramebuffer(uint32_t width, uint32_t height,
                                      bool depthOnly = false);

        FramebufferChain createFramebufferSet(uint32_t width,
                                            uint32_t height,
                                            bool depthOnly = false,
                                            uint32_t count = 3);

        FramebufferChain getDefaultFramebufferChain();

        ResourceImage* createResourceImage(uint32_t width, uint32_t height);

        Texture* createTexture(uint32_t width, uint32_t height, const TextureSetup& setup = TextureSetup());

        SemaphoreChain* createSemaphoreChain();

        void dispatchCompute(ComputePipeline* compute,
                             const std::initializer_list<ResourceSet*>& resource_sets,
                             const std::initializer_list<SemaphoreChain*>& semaphores, int x_dim = 1, int y_dim = 1, int z_dim = 1);

        void setPresentWaitForSemaphores(const internal::SemaphoreSet& semaphores);
        Semaphore createAndAddImageReadySemaphore();

        void present();

        void destroy(Pipeline* pipeline);
        void destroy(RenderFamily* family);
        void destroy(Buffer* buffer);
        void destroy(Shader* shader);
        void destroy(Texture* texture);
        void destroy(StorageBuffer* storageBuffer);
        void destroy(SemaphoreChain* semaphore_chain);
        void destroy(ResourceImage* resource_image);
        void destroy(ResourceSet* resource_set);
        void destroy(ComputePipeline* compute_pipeline);

        template<typename Type>
        void destroy(Uniform<Type>* uniform);

        Wingine(Winval& win);

        Wingine(int width, int height,
                winval_type_0 arg0,
                winval_type_1 arg1,
                const char* str = "Wingine");

        Wingine(int width, int height, const std::string& app_name = "Wingine");

        ~Wingine();

        friend class Buffer;
        friend class Pipeline;
        friend class RenderFamily;
        friend class ResourceSetLayout;
        friend class ResourceSet;
        friend class Texture;
        friend class SemaphoreChain;
        friend class Image;
        friend class ComputePipeline;
        friend class ResourceImage;
    };


    // Template things

    template<typename Type>
    VertexBuffer<Type>* Wingine::createVertexBuffer(uint32_t num, bool host_updatable) {
        return new VertexBuffer<Type>(*this, num, host_updatable);
    }

    template<typename Type>
    Uniform<Type>* Wingine::createUniform() {
        return new Uniform<Type>(*this);
    }


    template<typename Type>
    void Wingine::destroy(Uniform<Type>* uniform) {
        this->destroy(uniform->buffer);
        delete uniform->buffer_info;
        delete uniform;
    }
};
