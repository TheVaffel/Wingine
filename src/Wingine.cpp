#include "./Wingine.hpp"
#include "./util/log.hpp"
#include "./constants.hpp"

#include "./framebuffer/BasicFramebuffer.hpp"
#include "./framebuffer/HostCopyingFramebufferChain.hpp"

#include "./image/BasicImage.hpp"
#include "./image/ImageCopier.hpp"

#include "./draw_pass/BasicDrawPass.hpp"

#include "./resource/BasicResourceSetChain.hpp"

#include "./image/BasicTextureChain.hpp"

#include "./pipeline/BasicShader.hpp"
#include "./pipeline/BasicPipeline.hpp"

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

    void Wingine::cmd_set_layout(const vk::CommandBuffer& commandBuffer, vk::Image image,
                                 vk::ImageAspectFlagBits aspect, vk::ImageLayout currentLayout,
                                 vk::ImageLayout finalLayout) {
        vk::ImageMemoryBarrier image_memory_barrier;

        vk::ImageSubresourceRange sbr;
        sbr.setAspectMask(aspect)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        image_memory_barrier.setOldLayout(currentLayout)
            .setNewLayout(finalLayout)
            .setImage(image)
            .setSubresourceRange(sbr);

        vk::PipelineStageFlags srcStage, destStage;

        switch(currentLayout) {
        case vk::ImageLayout::eColorAttachmentOptimal:
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            break;
        case vk::ImageLayout::eTransferDstOptimal:
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
            srcStage = vk::PipelineStageFlagBits::eTransfer;
            break;
        case vk::ImageLayout::ePreinitialized:
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite);
            srcStage = vk::PipelineStageFlagBits::eHost;
            break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
            srcStage = vk::PipelineStageFlagBits::eComputeShader |
                vk::PipelineStageFlagBits::eFragmentShader;
            break;
        case vk::ImageLayout::eTransferSrcOptimal:
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
            srcStage = vk::PipelineStageFlagBits::eTransfer;
            break;
        default:
            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        }

        switch (finalLayout) {
        case vk::ImageLayout::eTransferDstOptimal:
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
            destStage = vk::PipelineStageFlagBits::eTransfer;
            break;
        case vk::ImageLayout::eTransferSrcOptimal:
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
            destStage = vk::PipelineStageFlagBits::eTransfer;
            break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            destStage = vk::PipelineStageFlagBits::eComputeShader |
                vk::PipelineStageFlagBits::eFragmentShader;
            break;
        case vk::ImageLayout::eColorAttachmentOptimal:
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            destStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
            destStage = vk::PipelineStageFlagBits::eLateFragmentTests;
            break;
        default:
            destStage = vk::PipelineStageFlagBits::eBottomOfPipe;
        }

        commandBuffer.pipelineBarrier(srcStage, destStage, {}, 0,  nullptr, 0, nullptr, 1, &image_memory_barrier);
    }


    void Wingine::copy_image(uint32_t w1, uint32_t h1, vk::Image src,
                             vk::ImageLayout srcCurrentLayout, vk::ImageLayout srcFinalLayout,
                             uint32_t w2, uint32_t h2, vk::Image dst,
                             vk::ImageLayout dstCurrentLayout, vk::ImageLayout dstFinalLayout,
                             vk::ImageAspectFlagBits aspect,
                             const std::initializer_list<SemaphoreChain*>& semaphores) {

        const vk::CommandBuffer& command_buffer = this->command_manager->getGeneralCommand().buffer;
        const vk::Fence& command_fence = this->command_manager->getGeneralCommand().fence;

        vk::CommandBufferBeginInfo bg;
        _wassert_result(this->device.waitForFences(1, &command_fence,
                                                   true, (uint64_t)1e9),
                        "wait for general purpose command in copy_image to finish");

        vk::Result res = this->device.resetFences(1, &command_fence);
        _wassert_result(res, "reset fence in copy_image");

        command_buffer.begin(bg);

        cmd_set_layout(command_buffer, src,
                       aspect, srcCurrentLayout, vk::ImageLayout::eTransferSrcOptimal);

        cmd_set_layout(command_buffer, dst,
                       aspect, dstCurrentLayout, vk::ImageLayout::eTransferDstOptimal);

        if (aspect == vk::ImageAspectFlagBits::eDepth) {
            _wassert(w1 == w2 && h1 == h2,
                     "Cannot copy depth image where source and destination have different extents");

            vk::ImageCopy copy;

            vk::ImageSubresourceLayers subr;
            subr.setAspectMask(aspect)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            vk::Offset3D offs;
            offs.setX(0)
                .setY(0)
                .setZ(0);

            vk::Extent3D ext;
            ext.setWidth(w1)
                .setHeight(h1)
                .setDepth(1);

            copy.setSrcSubresource(subr)
                .setSrcOffset(offs)
                .setDstSubresource(subr)
                .setDstOffset(offs)
                .setExtent(ext);

            command_buffer.copyImage(src, vk::ImageLayout::eTransferSrcOptimal,
                                     dst, vk::ImageLayout::eTransferDstOptimal,
                                     1, &copy);
        } else {
            vk::ImageBlit blit;
            blit.srcSubresource.setAspectMask(aspect)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);
            blit.setSrcOffsets({vk::Offset3D{0, 0, 0}, vk::Offset3D{(int)w1, (int)h1, 1}});

            blit.dstSubresource.setAspectMask(aspect)
                .setMipLevel(0)
                .setBaseArrayLayer(0)
                .setLayerCount(1);
            blit.setDstOffsets({vk::Offset3D{0, 0, 0}, vk::Offset3D{(int)w2, (int)h2, 1}});

            vk::Filter filter = (aspect == vk::ImageAspectFlagBits::eDepth ||
                                 aspect == vk::ImageAspectFlagBits::eStencil) ?
                vk::Filter::eNearest : vk::Filter::eLinear;

            command_buffer.blitImage(src, vk::ImageLayout::eTransferSrcOptimal,
                                                     dst, vk::ImageLayout::eTransferDstOptimal,
                                                     1, &blit, filter);
        }

        // Perhaps not necessary, but very convenient
        cmd_set_layout(command_buffer, src,
                       aspect, vk::ImageLayout::eTransferSrcOptimal, srcFinalLayout);

        cmd_set_layout(command_buffer, dst,
                       aspect, vk::ImageLayout::eTransferDstOptimal, dstFinalLayout);

        command_buffer.end();


        std::vector<vk::Semaphore> wait_sems(semaphores.size());
        std::vector<vk::Semaphore> signal_sems(semaphores.size());
        std::vector<uint64_t> wait_vals(semaphores.size());
        std::vector<uint64_t> signal_vals(semaphores.size());
        std::vector<vk::PipelineStageFlags> flags(semaphores.size());

        int num_wait_sems = SemaphoreChain::getWaitSemaphores(wait_sems.data(), std::begin(semaphores), semaphores.size());
        int num_sig_sems = SemaphoreChain::getSignalSemaphores(signal_sems.data(), std::begin(semaphores), semaphores.size());

        SemaphoreChain::getSemaphoreWaitValues(wait_vals.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getSemaphoreSignalValues(signal_vals.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getWaitStages(flags.data(), std::begin(semaphores), semaphores.size());

        vk::TimelineSemaphoreSubmitInfo tssi;
        tssi.setWaitSemaphoreValueCount(num_wait_sems)
            .setPWaitSemaphoreValues(wait_vals.data())
            .setSignalSemaphoreValueCount(num_sig_sems)
            .setPSignalSemaphoreValues(signal_vals.data());

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&command_buffer)
            .setPWaitDstStageMask(flags.data())
            .setWaitSemaphoreCount(num_wait_sems)
            .setPWaitSemaphores(wait_sems.data())
            .setSignalSemaphoreCount(num_sig_sems)
            .setPSignalSemaphores(signal_sems.data())
            .setPNext(&tssi);

        SemaphoreChain::resetModifiers(std::begin(semaphores), semaphores.size());

        _wassert_result(this->queue_manager->getGraphicsQueue().submit(1, &si, command_fence),
                        "command submission to graphics queue in copy_image");


        if (semaphores.size() == 0) {
            // If we don't wait for it to finish, we cannot guarantee that it is actually ready for use
            _wassert_result(this->device.waitForFences(1, &command_fence, true, (uint64_t)1e9),
                            "wait for general purpose command in end of copy_image");
        }

    }

    vk::RenderPass Wingine::create_render_pass(internal::renderPassUtil::RenderPassType type,
                                               bool clear) {

        std::vector<vk::AttachmentDescription> descriptions;
        std::vector<vk::AttachmentReference> references;

        switch(type) {
        case internal::renderPassUtil::RenderPassType::colorDepth:
            descriptions.resize(2);
            descriptions[0].setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                .setInitialLayout(vk::ImageLayout::ePresentSrcKHR)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
                .setFormat(vk::Format::eB8G8R8A8Unorm);
            descriptions[1].setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFormat(vk::Format::eD32Sfloat);
            references.resize(2);
            references[0].setAttachment(0)
                .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
            references[1].setAttachment(1)
                .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
            break;
        case internal::renderPassUtil::RenderPassType::depthOnly:
            descriptions.resize(1);
            descriptions[0].setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFormat(vk::Format::eD32Sfloat);
            references.resize(1);
            references[0].setAttachment(0)
                .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
            break;
        }

        if (clear) {
            for(vk::AttachmentDescription& ds : descriptions) {
                ds.setLoadOp(vk::AttachmentLoadOp::eClear);
                ds.setStencilLoadOp(vk::AttachmentLoadOp::eClear);
                ds.setInitialLayout(vk::ImageLayout::eUndefined);
            }
        }

        vk::SubpassDescription spd;
        spd.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
        if(type == internal::renderPassUtil::RenderPassType::colorDepth) {

            spd.setColorAttachmentCount(1)
                .setPColorAttachments(references.data())
                .setPDepthStencilAttachment(references.data() + 1);
        } else if (type == internal::renderPassUtil::RenderPassType::depthOnly) {
            spd.setColorAttachmentCount(0)
                .setPDepthStencilAttachment(references.data());
        }

        vk::RenderPassCreateInfo rpci;
        rpci.setAttachmentCount(descriptions.size())
            .setPAttachments(descriptions.data())
            .setSubpassCount(1)
            .setPSubpasses(&spd);

        return this->device.createRenderPass(rpci);
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

        /* this->staging_buffer_manager =
            std::make_shared<internal::StagingBufferManager>(this->command_manager,
                                                             this->queue_manager,
                                                             const_device_manager); */

        // General purpose fence for the moving stage
        vk::FenceCreateInfo fence_create_info;
        fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

        this->general_purpose_fence =
            this->device.createFence(fence_create_info);

        this->init_generic_render_pass();

        this->init_descriptor_pool();

        this->init_pipeline_cache();
    }

    void Wingine::cons_image_image(Image& image, uint32_t width, uint32_t height,
                                   vk::Format format, vk::ImageUsageFlags usage,
                                   vk::ImageTiling tiling,
                                   vk::ImageLayout layout) {
        vk::ImageCreateInfo inf;
        inf.setExtent({width, height, 1})
            .setImageType(vk::ImageType::e2D)
            .setTiling(tiling)
            .setInitialLayout(layout)
            .setUsage(usage)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setFormat(format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setMipLevels(1)
            .setArrayLayers(1);

        image.width = width;
        image.height = height;

        image.current_layout = inf.initialLayout;

        image.image = this->device.createImage(inf);
    }

    void Wingine::cons_image_memory(Image& image,
                                    vk::MemoryPropertyFlags memProps) {
        vk::MemoryAllocateInfo mai;

        vk::MemoryRequirements mr;
        mr = this->device.getImageMemoryRequirements(image.image);

        mai.allocationSize = mr.size;
        mai.memoryTypeIndex = _get_memory_type_index(mr.memoryTypeBits,
                                                     memProps,
                                                     this->device_manager->getDeviceMemoryProperties());
        image.memory = this->device.allocateMemory(mai);
        this->device.bindImageMemory(image.image, image.memory, 0); // 0 offset from memory start
    }

    void Wingine::cons_image_view(Image& image,
                                  ImageViewType type,
                                  vk::Format format) {
        vk::ImageViewCreateInfo ivci;
        ivci.setImage(image.image)
            .setViewType(vk::ImageViewType::e2D)
            .setComponents({vk::ComponentSwizzle::eR,
                            vk::ComponentSwizzle::eG,
                            vk::ComponentSwizzle::eB,
                            vk::ComponentSwizzle::eA});

        switch(type) {
        case wImageViewColor:
            ivci.setFormat(format)
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor,
                                      0, 1, 0, 1});
            break;
        case wImageViewDepth:
            ivci.setFormat(vk::Format::eD32Sfloat)
                .setSubresourceRange({vk::ImageAspectFlagBits::eDepth,
                                      0, 1, 0, 1});
            break;
        }

        image.view = this->device.createImageView(ivci);
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

    ComputePipeline* Wingine::createComputePipeline(const std::vector<std::vector<uint64_t> >& resourceSetLayouts,
                                                    Shader* shader) {
        std::vector<vk::DescriptorSetLayout> rsl;
        for(unsigned int i = 0; i < resourceSetLayouts.size(); i++) {
            rsl.push_back(this->resource_set_layout_registry->ensureAndGet(resourceSetLayouts[i]));
        }

        return new ComputePipeline(*this,
                                   rsl,
                                   shader);
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

    ResourceImage* Wingine::createResourceImage(uint32_t width, uint32_t height) {
        ResourceImage* image = new ResourceImage(*this, width, height);

        return image;
    }

    TexturePtr Wingine::createBasicTexture(uint32_t width, uint32_t height, const BasicTextureSetup& setup) {
        return std::make_shared<internal::BasicTexture>(vk::Extent2D(width, height),
                                                        setup,
                                                        this->device_manager);
    }

    TextureChainPtr Wingine::createBasicTextureChain(uint32_t width,
                                                     uint32_t height,
                                                     const BasicTextureSetup& setup) {
        return std::make_shared<internal::BasicTextureChain>(this->getNumFramebuffers(),
                                                             vk::Extent2D(width, height),
                                                             setup,
                                                             this->device_manager);
    }

    ResourceSetChainPtr Wingine::createResourceSetChain(const std::vector<uint64_t>& resourceLayout) {
        return std::make_shared<internal::BasicResourceSetChain>(
            this->getNumFramebuffers(),
            this->resource_set_layout_registry->ensureAndGet(resourceLayout),
            this->descriptor_pool,
            this->device_manager);
    }

    SemaphoreChain* Wingine::createSemaphoreChain() {
        SemaphoreChain* semaphore_chain = new SemaphoreChain(*this);
        return semaphore_chain;
    }

    EventChainPtr Wingine::createEventChain() {
        EventChainPtr events = std::make_shared<internal::EventChain>(this->getNumFramebuffers(),
                                                                      this->device_manager);
        return events;
    }

    /* StorageBuffer* Wingine::createStorageBuffer(uint32_t num_bytes, bool host_updatable) {
        return new StorageBuffer(*this,
                                 num_bytes, host_updatable);
                                 } */

    DrawPassPtr Wingine::createBasicDrawPass(PipelinePtr pipeline,
                                             const internal::BasicDrawPassSettings& settings) {
        return std::make_shared<internal::BasicDrawPass>(pipeline,
                                                         this->getDefaultFramebufferChain()->getNumFramebuffers(),
                                                         settings,
                                                         this->command_manager,
                                                         this->queue_manager,
                                                         this->device_manager);

    }

    void Wingine::dispatchCompute(ComputePipeline* compute,
                                  const std::initializer_list<ResourceSet*>& resource_sets,
                                  const std::initializer_list<SemaphoreChain*>& semaphores, int x_dim, int y_dim, int z_dim) {
        compute->command.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        vk::CommandBufferBeginInfo begin;
        compute->command.buffer.begin(begin);
        compute->command.buffer.bindPipeline(vk::PipelineBindPoint::eCompute,
                                             compute->pipeline);

        std::vector<vk::DescriptorSet> d_sets(resource_sets.size());
        for(unsigned int i = 0; i < d_sets.size(); i++) {
            d_sets[i] = std::begin(resource_sets)[i]->descriptor_set;
        }

        if(resource_sets.size()) {
            compute->command.buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                       compute->layout,
                                                       0, d_sets.size(),
                                                       d_sets.data(),
                                                       0, nullptr);
        }

        compute->command.buffer.dispatch(x_dim, y_dim, z_dim);
        compute->command.buffer.end();


        std::vector<vk::PipelineStageFlags> stage_flags(semaphores.size());
        std::vector<vk::Semaphore> wait_sems(semaphores.size());
        std::vector<vk::Semaphore> signal_sems(semaphores.size());
        std::vector<uint64_t> wait_vals(semaphores.size());
        std::vector<uint64_t> signal_vals(semaphores.size());

        int num_wait_sems = SemaphoreChain::getWaitSemaphores(wait_sems.data(), std::begin(semaphores), semaphores.size());
        int num_signal_sems = SemaphoreChain::getSignalSemaphores(signal_sems.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getSemaphoreWaitValues(wait_vals.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getSemaphoreSignalValues(signal_vals.data(), std::begin(semaphores), semaphores.size());
        SemaphoreChain::getWaitStages(stage_flags.data(), std::begin(semaphores), semaphores.size());

        vk::TimelineSemaphoreSubmitInfo tssi;
        tssi.setSignalSemaphoreValueCount(num_signal_sems)
            .setPSignalSemaphoreValues(signal_vals.data())
            .setWaitSemaphoreValueCount(num_wait_sems)
            .setPWaitSemaphoreValues(wait_vals.data());

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&compute->command.buffer)
            .setPWaitDstStageMask(stage_flags.data())
            .setSignalSemaphoreCount(num_signal_sems)
            .setPSignalSemaphores(signal_sems.data())
            .setWaitSemaphoreCount(num_wait_sems)
            .setPWaitSemaphores(wait_sems.data())
            .setPNext(&tssi);

        SemaphoreChain::resetModifiers(std::begin(semaphores), semaphores.size());

        _wassert_result(device.waitForFences(1, &compute->command.fence, true,
                                             (uint64_t)1e9),
                        "wait for last submission in compute submit");
        _wassert_result(this->device.resetFences(1, &compute->command.fence),
                        "reset fence at end of compute submit");

        _wassert_result(this->queue_manager->getComputeQueue().submit(1, &si, compute->command.fence),
                        "submitting compute command");
    }

    void Wingine::destroySwapchainImage(Image& image) {
        this->device.free(image.memory, nullptr);
        this->device.destroy(image.view, nullptr);
    }

    void Wingine::destroy(ResourceImage* resourceImage) {
        this->device.free(resourceImage->memory, nullptr);
        this->device.destroy(resourceImage->view, nullptr);
        this->device.destroy(resourceImage->image, nullptr);

        delete resourceImage->image_info;
        delete resourceImage;
    }

    void Wingine::destroy(ResourceSet* resource_set) {
        delete resource_set;
    }

    void Wingine::destroy(ComputePipeline* compute_pipeline) {
        this->device.destroy(compute_pipeline->layout, nullptr);
        this->device.destroy(compute_pipeline->pipeline, nullptr);

        this->device.destroy(compute_pipeline->command.fence, nullptr);
        this->device.freeCommandBuffers(this->command_manager->getGraphicsCommandPool(),
                                        1, &compute_pipeline->command.buffer);

        delete compute_pipeline;
    }

    void Wingine::destroy(SemaphoreChain* semaphore_chain) {
        this->waitIdle();
        _wassert_result(this->device.resetFences(1, &this->general_purpose_fence),
                        "reset fence in destroying semaphore chain");

                        semaphore_chain->ensure_finished(this, this->general_purpose_fence);
        this->device.destroy(semaphore_chain->semaphore, nullptr);

        delete semaphore_chain;
    }

    void Wingine::destroy(Buffer* buffer) {
        this->device.destroy(buffer->buffer, nullptr);
        this->device.free(buffer->memory, nullptr);

        if( !buffer->host_updatable ) {
            this->device.destroy(buffer->update_buffer, nullptr);
            this->device.free(buffer->update_memory, nullptr);
        }

        delete buffer;
    }

    void Wingine::destroy(Image& image) {
        this->device.destroy(image.image, nullptr);
        this->device.free(image.memory, nullptr);
        this->device.destroy(image.view, nullptr);
    }

    /* void Wingine::destroy(StorageBuffer* storagebuffer) {
        this->destroy(storagebuffer->buffer);
        delete storagebuffer->buffer_info;
        delete storagebuffer;
        } */

    Wingine::~Wingine() {
        this->device.destroy(this->descriptor_pool, nullptr);
        this->device.destroy(this->pipeline_cache, nullptr);

        this->device.destroyFence(this->general_purpose_fence);
    }
};
