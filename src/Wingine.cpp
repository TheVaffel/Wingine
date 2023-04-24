#include "./Wingine.hpp"
#include "./util/log.hpp"
#include "./core/constants.hpp"

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

#include "./resource/StaticResourceChain.hpp"
#include "./resource/BasicRawUniformChain.hpp"

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

    void Wingine::setPresentWaitForSemaphores(internal::WaitSemaphoreSet&& semaphores) {
        this->default_framebuffer_chain->setPresentWaitSemaphores(std::move(semaphores));
    }

    void Wingine::present() {
        this->default_framebuffer_chain->swapToNextElement();
        this->default_chain_reel->swap();
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
        this->compatible_render_pass_registry->ensureAndGetRenderPass(
            internal::renderPassUtil::RenderPassType::colorDepth);
    }


    void Wingine::init_descriptor_pool() {
        const int max_num_descriptors = 1000; // Use as placeholder - refactor this part if necessary

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

    void Wingine::setChainReel(ChainReelPtr chain_reel) {
        this->current_chain_reel = chain_reel;
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

        this->default_chain_reel =
            std::make_shared<internal::ChainReel>(this->getNumFramebuffers());
        this->current_chain_reel = this->default_chain_reel;

        this->init_generic_render_pass();

        this->init_descriptor_pool();

        this->init_pipeline_cache();
    }

#ifndef HEADLESS
    Wingine::Wingine(uint32_t width, uint32_t height,
                     winval_type_0 arg0, winval_type_1 arg1, const std::string& application_name) {
        VulkanInitInfo init_info(width, height, arg0, arg1, application_name);
        this->init_vulkan(init_info);
        this->original_device_manager_refs = this->device_manager.use_count();
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
        this->original_device_manager_refs = this->device_manager.use_count();
    }
#endif

    Wingine::Wingine(uint32_t width, uint32_t height, const std::string& app_title) {
        VulkanInitInfo init_info(width, height, app_title);
        this->init_vulkan(init_info);

        this->original_device_manager_refs = this->device_manager.use_count();
    }

    int Wingine::getNumFramebuffers() {
        return this->default_framebuffer_chain->getElementChainLength();
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

    ChainReelPtr Wingine::createChainReel(uint32_t chain_length) {
        return std::make_shared<internal::ChainReel>(chain_length);
    }

    ChainReelPtr Wingine::getDefaultChainReel() {
        return this->default_chain_reel;
    }

    ChainReelPtr Wingine::getCurrentChainReel() {
        return this->current_chain_reel;
    }

    Wingine::ChainReelGuard::ChainReelGuard(Wingine* wing, ChainReelPtr chain_reel)
        : wing(wing) {
        this->old_chain_reel = wing->getCurrentChainReel();
        wing->setChainReel(chain_reel);
    }

    Wingine::ChainReelGuard::~ChainReelGuard() {
        this->wing->setChainReel(this->old_chain_reel);
    }

    std::unique_ptr<Wingine::ChainReelGuard> Wingine::lockChainReel(ChainReelPtr chain_reel) {
        return std::unique_ptr<Wingine::ChainReelGuard>(new ChainReelGuard(this, chain_reel));
    }

    ShaderPtr Wingine::createShader(internal::ShaderStage stage_bit, const std::vector<uint32_t>& spirv) {
        return std::make_shared<internal::BasicShader>(stage_bit, std::move(spirv), this->device_manager);
    }

    PipelinePtr Wingine::createBasicPipeline(const std::vector<VertexAttribDesc>& descriptions,
                                             const std::vector<ShaderPtr>& shaders,
                                             internal::BasicPipelineSetup setup) {
        if (setup.width == 0 || setup.height == 0) {
            vk::Extent2D dims = this->default_framebuffer_chain->getFramebuffer(0).getDimensions();
            setup.width = dims.width;
            setup.height = dims.height;
        }

        return std::make_shared<internal::BasicPipeline>(setup,
                                                         descriptions,
                                                         shaders,
                                                         this->device_manager,
                                                         this->compatible_render_pass_registry,
                                                         this->pipeline_cache);
    }

    ComputePipelinePtr Wingine::createComputePipeline(ShaderPtr shader) {
        return std::make_shared<internal::BasicComputePipeline>(shader,
                                                                this->descriptor_pool,
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

    FramebufferChain Wingine::createFramebufferChain(uint32_t width, uint32_t height, bool depthOnly) {
        internal::BasicFramebufferSetup setup;
        setup.setDepthOnly(depthOnly);
        auto framebuffer_chain =
            std::make_shared<internal::BasicFramebufferChain<
                internal::BasicFramebuffer>>(this->current_chain_reel->getChainLength(),
                                             this->device_manager,
                                             this->queue_manager,
                                             vk::Extent2D(width, height),
                                             setup,
                                             this->device_manager,
                                             *this->compatible_render_pass_registry);

        this->current_chain_reel->addChain(framebuffer_chain);

        return framebuffer_chain;
    }

    FramebufferTextureChainPtr Wingine::createFramebufferTextureChain(uint32_t width,
                                                                      uint32_t height,
                                                                      bool depth_only) {

        auto framebuffer_chain =  std::make_shared<internal::FramebufferTextureChain>(
            this->current_chain_reel->getChainLength(),
            vk::Extent2D(width, height),
            depth_only,
            this->device_manager,
            this->queue_manager,
            *this->compatible_render_pass_registry);

        this->current_chain_reel->addChain(framebuffer_chain);

        return framebuffer_chain;
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
                                                         this->getNumFramebuffers(),
                                                         settings,
                                                         this->descriptor_pool,
                                                         this->command_manager,
                                                         this->queue_manager,
                                                         this->device_manager);

    }

    RawUniformChainPtr Wingine::createRawUniformChain(uint32_t byte_size) {
        auto uniform_chain = std::make_shared<internal::BasicRawUniformChain>(this->getNumFramebuffers(),
                                                                              byte_size,
                                                                              this->device_manager);
        this->current_chain_reel->addChain(uniform_chain);
        return uniform_chain;
    }

    ResourceChainPtr Wingine::createResourceChain(std::shared_ptr<internal::IResource> resource) {
        return std::make_shared<internal::StaticResourceChain>(this->getNumFramebuffers(), resource);
    }

    void Wingine::clearAndCheckDeviceManagerRefs() {
        this->current_chain_reel->reset();
        this->default_chain_reel->reset();
        this->default_framebuffer_chain->setSignalImageAcquiredSemaphores({});
        this->default_framebuffer_chain->setPresentWaitSemaphores({});

        if (this->original_device_manager_refs != this->device_manager.use_count()) {
            std::cerr << "Wingine device manager ref count was wrong, expected "
                      << this->original_device_manager_refs << ", got "
                      << this->device_manager.use_count() << std::endl;
            throw std::runtime_error("Wrong number of device_manager references. Have you forgotten to manually destruct an object in C API?");
        }
    }

    Wingine::~Wingine() {
        this->device.destroy(this->descriptor_pool, nullptr);
        this->device.destroy(this->pipeline_cache, nullptr);

        this->clearAndCheckDeviceManagerRefs();
    }
};
