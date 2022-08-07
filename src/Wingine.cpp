#include "./Wingine.hpp"
#include "./util/log.hpp"
#include "./constants.hpp"

#include "./framebuffer/BasicFramebuffer.hpp"
#include "./framebuffer/HostCopyingFramebufferChain.hpp"

#include "./image/BasicImage.hpp"
#include "./image/ImageCopier.hpp"

#include "./draw_pass/BasicDrawPass.hpp"

#include "./image/BasicTextureChain.hpp"
#include "./image/InternallyStagedTexture.hpp"

#include "./pipeline/BasicShader.hpp"
#include "./pipeline/BasicPipeline.hpp"
#include "./pipeline/BasicComputePipeline.hpp"

#include "./buffer/InternallyStagedIndexBuffer.hpp"

#include <exception>


/*
 * O boi here we go
 */

namespace wg {

    /*
     * VulkanInitInfo
     */

    const vk::Extent2D& Wingine::VulkanInitInfo::getDimensions() const {
        return this->dimensions;
    }

    std::string Wingine::VulkanInitInfo::getApplicationName() const {
        return this->application_name;
    }

    bool Wingine::VulkanInitInfo::getIsHeadless() const {
        return this->is_headless;
    }

#ifndef HEADLESS
    winval_type_0 Wingine::VulkanInitInfo::getWinArg0() const {
        return this->win_arg0;
    }

    winval_type_1 Wingine::VulkanInitInfo::getWinArg1() const {
        return this->win_arg1;
    }
#endif

    Wingine::VulkanInitInfo::VulkanInitInfo(uint32_t width, uint32_t height, const std::string& application_name)
        : dimensions(width, height),
          application_name(application_name),
          is_headless(true) { }

#ifndef HEADLESS
    Wingine::VulkanInitInfo::VulkanInitInfo(uint32_t width,
                                            uint32_t height,
                                            winval_type_0 arg0,
                                            winval_type_1 arg1,
                                            const std::string& application_name)
        : dimensions(width, height),
          application_name(application_name),
          win_arg0(arg0),
          win_arg1(arg1) { }
#endif

    /*
     * Wingine
     */

    int Wingine::getWindowWidth() {
        return this->window_width;
    }

    int Wingine::getWindowHeight() {
        return this->window_height;
    }

    void Wingine::register_compatible_render_pass(internal::renderPassUtil::RenderPassType type) {
        this->compatible_render_pass_registry->ensureAndGetRenderPass(type);
    }

    void Wingine::setPresentWaitForSemaphores(internal::WaitSemaphoreSet&& semaphores) {
        this->default_framebuffer_chain->setPresentWaitSemaphores(std::move(semaphores));
    }

    void Wingine::present() {
        this->default_framebuffer_chain->swapFramebuffer();
    }

    uint32_t Wingine::getRenderedImageRowByteStride() const {
        if (!this->host_visible_image) {
            throw std::runtime_error("[Wingine] Image copying not enabled (not in headless mode?)");
        }

        return this->host_visible_image->getByteStride();
    }

    void Wingine::copyLastRenderedImage(uint32_t* dst) {
        if (!this->host_visible_image) {
            throw std::runtime_error("[Wingine] Image copying not enabled (not in headless mode?)");
        }
        this->host_visible_image->copyImageToHost(dst);
    }

    Semaphore Wingine::createAndAddImageReadySemaphore() {
        return this->default_framebuffer_chain->addSignalImageAcquiredSemaphore();
    }

    void Wingine::setImageReadySemaphores(internal::SignalSemaphoreSet&& semaphores) {
        return this->default_framebuffer_chain->setSignalImageAcquiredSemaphores(std::move(semaphores));
    }

    void Wingine::init_generic_render_pass() {
        this->register_compatible_render_pass(internal::renderPassUtil::RenderPassType::colorDepth);
    }


    void Wingine::init_descriptor_pool() {
        const int max_num_descriptors = 16; // Use as placeholder - refactor this part if necessary

        std::vector<vk::DescriptorPoolSize> sizes(4);
        sizes[0].setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(max_num_descriptors);

        sizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(max_num_descriptors);

        sizes[2].setType(vk::DescriptorType::eStorageImage)
            .setDescriptorCount(max_num_descriptors);

        sizes[3].setType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(max_num_descriptors);

        vk::DescriptorPoolCreateInfo dpi;
        dpi.setMaxSets(3 * max_num_descriptors)
            .setPoolSizeCount(sizes.size())
            .setPPoolSizes(sizes.data());

        this->descriptor_pool = this->device.createDescriptorPool(dpi);

    }

    void Wingine::init_pipeline_cache() {
        vk::PipelineCacheCreateInfo pcci;

        // We use defaults in the create info
        this->pipeline_cache = this->device.createPipelineCache(pcci);
    }

    void Wingine::waitIdle() {
	this->queue_manager->getGraphicsQueue().waitIdle();

        if (this->queue_manager->hasPresentQueue()) {
            this->queue_manager->getPresentQueue().waitIdle();
        }

        if (this->queue_manager->hasComputeQueue()) {
            this->queue_manager->getComputeQueue().waitIdle();
        }
    }

    void Wingine::init_vulkan(const VulkanInitInfo& init_info) {

        this->window_width = init_info.getDimensions().width;
        this->window_height = init_info.getDimensions().height;

        if (init_info.getIsHeadless()) {
            this->vulkan_instance_manager =
                std::make_shared<internal::VulkanInstanceManager>(init_info.getApplicationName());
        }

#ifndef HEADLESS
        else {
            this->vulkan_instance_manager =
                std::make_shared<internal::VulkanInstanceManager>(init_info.getWinArg0(),
                                                                  init_info.getWinArg1(),
                                                                  init_info.getApplicationName());
        }
#endif

        this->device_manager =
            std::make_shared<internal::DeviceManager>(this->vulkan_instance_manager);

        this->device = this->device_manager->getDevice();

        std::shared_ptr<const internal::DeviceManager> const_device_manager =
            const_pointer_cast<const internal::DeviceManager>(this->device_manager);

        this->queue_manager =
            std::make_shared<internal::QueueManager>(const_device_manager,
                                                     *this->vulkan_instance_manager);

        std::shared_ptr<const internal::QueueManager> const_queue_manager =
            const_pointer_cast<const internal::QueueManager>(this->queue_manager);

        this->compatible_render_pass_registry =
            std::make_shared<internal::CompatibleRenderPassRegistry>(const_device_manager);

        this->command_manager =
            std::make_shared<internal::CommandManager>(const_device_manager,
                                                       const_queue_manager);

        std::shared_ptr<const internal::CommandManager> const_command_manager =
            const_pointer_cast<const internal::CommandManager>(this->command_manager);

        if (init_info.getIsHeadless()) {
            this->host_visible_image = std::make_shared<internal::HostVisibleImageView>(
                internal::constants::preferred_swapchain_image_count,
                init_info.getDimensions(),
                device_manager);

            this->default_framebuffer_chain =
                std::make_shared<internal::HostCopyingFramebufferChain>(
                    internal::constants::preferred_swapchain_image_count,
                    host_visible_image,
                    const_queue_manager,
                    const_command_manager,
                    const_device_manager,
                    *this->compatible_render_pass_registry);
        } else {
            this->default_framebuffer_chain =
                std::make_shared<internal::SwapchainFramebufferChain>(
                    init_info.getDimensions(),
                    this->vulkan_instance_manager->getSurface(),
                    const_device_manager,
                    const_queue_manager,
                    *this->compatible_render_pass_registry);
        }

        this->resource_set_layout_registry =
            std::make_shared<internal::ResourceSetLayoutRegistry>(this->device_manager);

        // General purpose fence for the moving stage
        vk::FenceCreateInfo fence_create_info;
        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->general_purpose_fence =
            this->device.createFence(fence_create_info);

        this->init_generic_render_pass();

        this->init_descriptor_pool();

        this->init_pipeline_cache();
    }

#ifndef HEADLESS
    Wingine::Wingine(uint32_t width, uint32_t height,
                     winval_type_0 arg0, winval_type_1 arg1, const std::string& application_name) {
        VulkanInitInfo init_info(width, height, arg0, arg1, application_name);
        this->init_vulkan(init_info);
    }
#endif

#ifndef HEADLESS
    Wingine::Wingine(Winval& win) {
#ifdef WIN32

        VulkanInitInfo init_info(win.getWidth(), win.getHeight(),
                                 win.getInstance(), win.getHWND(), win.getTitle());
        this->init_vulkan(init_info);

#else // WIN32

        VulkanInitInfo init_info(win.getWidth(), win.getHeight(),
                                 win.getWindow(), win.getDisplay(), win.getTitle());
        this->init_vulkan(init_info);

#endif // WIN32
    }
#endif

    Wingine::Wingine(uint32_t width, uint32_t height, const std::string& app_title) {
        VulkanInitInfo init_info(width, height, app_title);
        this->init_vulkan(init_info);
    }

    vk::Device Wingine::getDevice() {
        return this->device;
    }

    vk::Queue Wingine::getGraphicsQueue() {
        return this->queue_manager->getGraphicsQueue();
    }

    vk::Queue Wingine::getPresentQueue() {
        return this->queue_manager->getPresentQueue();
    }

    const internal::Command& Wingine::getGeneralCommand() {
        return this->command_manager->getGeneralCommand();
    }

    vk::CommandPool Wingine::getGraphicsCommandPool() {
        return this->command_manager->getGraphicsCommandPool();
    }

    vk::DescriptorPool Wingine::getDescriptorPool() {
        return this->descriptor_pool;
    }

    int Wingine::getNumFramebuffers() {
        return this->default_framebuffer_chain->getNumFramebuffers();
    }

    const internal::IFramebuffer& Wingine::getCurrentFramebuffer() {
        return this->default_framebuffer_chain->getCurrentFramebuffer();
    }

    IndexBufferPtr Wingine::createIndexBuffer(uint32_t num_indices) {
        return std::make_shared<internal::InternallyStagedIndexBuffer>(num_indices,
                                                                       this->device_manager,
                                                                       this->queue_manager,
                                                                       this->command_manager);
    }

    ShaderPtr Wingine::createShader(internal::ShaderStage stage_bit, const std::vector<uint32_t>& spirv) {
        return std::make_shared<internal::BasicShader>(stage_bit, std::move(spirv), this->device_manager);
    }

    PipelinePtr Wingine::createBasicPipeline(const std::vector<VertexAttribDesc>& descriptions,
                                             const std::vector<std::vector<uint64_t>>& resource_set_layout,
                                             const std::vector<ShaderPtr>& shaders,
                                             internal::BasicPipelineSetup setup) {
        if (setup.width == 0 || setup.height == 0) {
            vk::Extent2D dims = this->default_framebuffer_chain->getFramebuffer(0).getDimensions();
            setup.width = dims.width;
            setup.height = dims.height;
        }

        std::vector<vk::DescriptorSetLayout> rsl;
        for(unsigned int i = 0; i < resource_set_layout.size(); i++) {
            rsl.push_back(this->resource_set_layout_registry->ensureAndGet(resource_set_layout[i]));
        }

        return std::make_shared<internal::BasicPipeline>(setup,
                                                         descriptions,
                                                         rsl,
                                                         shaders,
                                                         this->device_manager,
                                                         this->compatible_render_pass_registry,
                                                         this->pipeline_cache);
    }

    ComputePipelinePtr Wingine::createComputePipeline(const std::vector<std::vector<uint64_t>>& resource_set_layouts,
                                                      ShaderPtr shader) {
        std::vector<vk::DescriptorSetLayout> rsl;
        for(unsigned int i = 0; i < resource_set_layouts.size(); i++) {
            rsl.push_back(this->resource_set_layout_registry->ensureAndGet(resource_set_layouts[i]));
        }

        return std::make_shared<internal::BasicComputePipeline>(rsl,
                                                                shader,
                                                                this->device_manager,
                                                                this->command_manager,
                                                                this->queue_manager,
                                                                this->pipeline_cache);
    }

    Framebuffer Wingine::createFramebuffer(uint32_t width, uint32_t height,
                                           bool depthOnly) {
        internal::BasicFramebufferSetup setup;
        setup.setDepthOnly(depthOnly);
        return std::make_unique<internal::BasicFramebuffer>(vk::Extent2D(width, height),
                                                            setup,
                                                            this->device_manager,
                                                            *this->compatible_render_pass_registry);
    }

    FramebufferChain Wingine::createFramebufferChain(uint32_t width, uint32_t height,
                                                     bool depthOnly, uint32_t num_framebuffers) {
        internal::BasicFramebufferSetup setup;
        setup.setDepthOnly(depthOnly);
        return std::make_shared<
            internal::BasicFramebufferChain<
                internal::BasicFramebuffer>>(num_framebuffers,
                                             this->device_manager,
                                             this->queue_manager,
                                             vk::Extent2D(width, height),
                                             setup,
                                             this->device_manager,
                                             *this->compatible_render_pass_registry);
    }

    FramebufferTextureChainPtr Wingine::createFramebufferTextureChain(uint32_t width,
                                                                      uint32_t height,
                                                                      bool depth_only,
                                                                      uint32_t count) {
        count = count == std::numeric_limits<uint32_t>::max() ? this->getNumFramebuffers() : count;
        return std::make_shared<internal::FramebufferTextureChain>(
            count,
            vk::Extent2D(width, height),
            depth_only,
            this->device_manager,
            this->queue_manager,
            *this->compatible_render_pass_registry);
    }

    FramebufferChain Wingine::getDefaultFramebufferChain() {
        return this->default_framebuffer_chain;
    }

    ImageCopierPtr Wingine::createImageCopier() {
        return std::make_unique<internal::ImageCopier>(this->queue_manager->getGraphicsQueue(),
                                                       this->command_manager,
                                                       this->device_manager);
    }

    ImageChainCopierPtr Wingine::createImageChainCopier() {
        return std::make_unique<internal::ImageChainCopier>(this->getNumFramebuffers(),
                                                            this->queue_manager->getGraphicsQueue(),
                                                            this->command_manager,
                                                            this->device_manager);
    }

    TexturePtr Wingine::createBasicTexture(uint32_t width, uint32_t height, const BasicTextureSetup& setup) {
        return std::make_shared<internal::BasicTexture>(vk::Extent2D(width, height),
                                                        setup,
                                                        this->device_manager);
    }

    SettableTexturePtr Wingine::createSettableTexture(uint32_t width, uint32_t height) {
        return std::make_shared<internal::InternallyStagedTexture>(vk::Extent2D(width, height),
                                                                   this->device_manager,
                                                                   this->queue_manager,
                                                                   this->command_manager);
    }

    StorageTexturePtr Wingine::createStorageTexture(uint32_t width, uint32_t height) {
        return internal::StorageTexture::createStorageTexture(vk::Extent2D(width, height),
                                                              this->device_manager,
                                                              this->command_manager,
                                                              this->queue_manager);
    }

    TextureChainPtr Wingine::createBasicTextureChain(uint32_t width,
                                                     uint32_t height,
                                                     const BasicTextureSetup& setup) {
        return std::make_shared<internal::BasicTextureChain>(this->getNumFramebuffers(),
                                                             vk::Extent2D(width, height),
                                                             setup,
                                                             this->device_manager);
    }

    EventChainPtr Wingine::createEventChain() {
        EventChainPtr events = std::make_shared<internal::EventChain>(this->getNumFramebuffers(),
                                                                      this->device_manager);
        return events;
    }

    DrawPassPtr Wingine::createBasicDrawPass(PipelinePtr pipeline,
                                             const internal::BasicDrawPassSettings& settings) {
        return std::make_shared<internal::BasicDrawPass>(pipeline,
                                                         this->getDefaultFramebufferChain()->getNumFramebuffers(),
                                                         settings,
                                                         this->command_manager,
                                                         this->queue_manager,
                                                         this->device_manager);

    }

    Wingine::~Wingine() {
        this->device.destroy(this->descriptor_pool, nullptr);
        this->device.destroy(this->pipeline_cache, nullptr);

        this->device.destroyFence(this->general_purpose_fence);
    }
};
