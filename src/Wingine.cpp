#include "Wingine.hpp"


/*
 * Debug callback
 */

static VKAPI_ATTR VkBool32 VKAPI_CALL
_debugCallbackFun(VkDebugReportFlagsEXT flags,
			 VkDebugReportObjectTypeEXT objectType,
			 uint64_t object,
			 size_t location, int32_t messageCode,
			 const char* pLayerPrefix, const char* pMessage,
			 void* pUserData) {
  
  std::cout << "[" << pLayerPrefix << "] Message: " << pMessage << std::endl;

  return false;
}

/*
 * O boi here we go
 */

namespace wg {

  int Wingine::getWindowWidth() {
    return this->window_width;
  }

  int Wingine::getWindowHeight() {
    return this->window_height;
  }
  
  void Wingine::cmd_set_layout(vk::CommandBuffer& commandBuffer, vk::Image image,
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
    vk::CommandBufferBeginInfo bg;
    this->device.waitForFences(1, &general_purpose_command.fence,
			       true, (uint64_t)1e9);

    this->device.resetFences(1, &general_purpose_command.fence);

    general_purpose_command.buffer.begin(bg);

    cmd_set_layout(general_purpose_command.buffer, src,
		   aspect, srcCurrentLayout, vk::ImageLayout::eTransferSrcOptimal);

    cmd_set_layout(general_purpose_command.buffer, dst,
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

      general_purpose_command.buffer.copyImage(src, vk::ImageLayout::eTransferSrcOptimal,
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

      general_purpose_command.buffer.blitImage(src, vk::ImageLayout::eTransferSrcOptimal,
					       dst, vk::ImageLayout::eTransferDstOptimal,
					       1, &blit, filter);
    }

    // Perhaps not necessary, but very convenient
    cmd_set_layout(general_purpose_command.buffer, src,
		   aspect, vk::ImageLayout::eTransferSrcOptimal, srcFinalLayout);

    cmd_set_layout(general_purpose_command.buffer, dst,
		   aspect, vk::ImageLayout::eTransferDstOptimal, dstFinalLayout);

    general_purpose_command.buffer.end();

    
    vk::Semaphore wait_sems[semaphores.size()];
    vk::Semaphore signal_sems[semaphores.size()];
    uint64_t wait_vals[semaphores.size()];
    uint64_t signal_vals[semaphores.size()];
    vk::PipelineStageFlags flags[semaphores.size()];

    int num_wait_sems = SemaphoreChain::getWaitSemaphores(wait_sems, std::begin(semaphores), semaphores.size());
    int num_sig_sems = SemaphoreChain::getSignalSemaphores(signal_sems, std::begin(semaphores), semaphores.size());
    
    SemaphoreChain::getSemaphoreWaitValues(wait_vals, std::begin(semaphores), semaphores.size());
    SemaphoreChain::getSemaphoreSignalValues(signal_vals, std::begin(semaphores), semaphores.size());
    SemaphoreChain::getWaitStages(flags, std::begin(semaphores), semaphores.size());

    vk::TimelineSemaphoreSubmitInfo tssi;
    tssi.setWaitSemaphoreValueCount(num_wait_sems)
      .setPWaitSemaphoreValues(wait_vals)
      .setSignalSemaphoreValueCount(num_sig_sems)
      .setPSignalSemaphoreValues(signal_vals);
    
    vk::SubmitInfo si;
    si.setCommandBufferCount(1)
      .setPCommandBuffers(&general_purpose_command.buffer)
      .setPWaitDstStageMask(flags)
      .setWaitSemaphoreCount(num_wait_sems)
      .setPWaitSemaphores(wait_sems)
      .setSignalSemaphoreCount(num_sig_sems)
      .setPSignalSemaphores(signal_sems)
      .setPNext(&tssi);

    SemaphoreChain::resetModifiers(std::begin(semaphores), semaphores.size());
    
    this->graphics_queue.submit(1, &si, general_purpose_command.fence);

    if (semaphores.size() == 0) {
      // If we don't wait for it to finish, we cannot guarantee that it is actually ready for use
      this->device.waitForFences(1, &general_purpose_command.fence, true, (uint64_t)1e9);
    }
    
  }
  
  vk::RenderPass Wingine::create_render_pass(RenderPassType type,
					     bool clear) {

    std::vector<vk::AttachmentDescription> descriptions;
    std::vector<vk::AttachmentReference> references;

    switch(type) {
    case renColorDepth:
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
    case renDepth:
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
    if(type == renColorDepth) {
      
      spd.setColorAttachmentCount(1)
	.setPColorAttachments(references.data())
	.setPDepthStencilAttachment(references.data() + 1);
    } else if (type == renDepth) {
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
					     
  
  void Wingine::register_compatible_render_pass(RenderPassType type) {
    this->compatibleRenderPassMap[type] =
      this->create_render_pass(type, false);
  }
  
  

  void Wingine::present(const std::initializer_list<SemaphoreChain*>& semaphores) {

#ifdef DEBUG
    if(!semaphores.size()) {
      std::cout << "[Wingine::present] Warning: No semaphore submitted to present(), presentation may not happen correctly"
		<< std::endl;
    }
#endif // DEBUG
    
    vk::PresentInfoKHR presentInfo;
    
    SemaphoreChain::chainsToSemaphore(this, std::begin(semaphores), semaphores.size(), this->finished_drawing_semaphore);

    // Present, but wait for finished_drawing_semaphore, which waits on the rest of the semaphores
    
    presentInfo.setSwapchainCount(1)
      .setPSwapchains(&this->swapchain)
      .setPImageIndices(&this->current_swapchain_image)
      .setWaitSemaphoreCount(1)
      .setPWaitSemaphores(&this->finished_drawing_semaphore)
      .setPResults(nullptr);
    
    this->present_queue.presentKHR(presentInfo);

    this->stage_next_image(semaphores);

    SemaphoreChain::resetModifiers(std::begin(semaphores), semaphores.size());
  }
  
  void Wingine::init_instance(int width, int height, const char* str) {
    std::vector<const char*> instance_extension_names;
    std::vector<const char*> instance_layer_names;

    instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef WIN32
    instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else // WIN32
    instance_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif // WIN32

#ifdef DEBUG
    instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");

    // instance_layer_names.push_back("VK_LAYER_LUNARG_api_dump");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_device_simulation");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_monitor");
    // instance_layer_names.push_back("VK_LAYER_RENDERDOC_Capture");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_api_dump");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_object_tracker");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_screenshot");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_starter_layer");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_parameter_validation");
    // instance_layer_names.push_back("VK_LAYER_GOOGLE_unique_objects");
    // instance_layer_names.push_back("VK_LAYER_LUNARG_vktrace");
    instance_layer_names.push_back("VK_LAYER_KHRONOS_validation");
    // instance_layer_names.push_back("VK_LAYER_GOOGLE_threading");
#endif // DEBUG

    uint32_t extension_count;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> props(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, props.data() + 0);
    /* for (unsigned int i = 0; i < extension_count; i++) {
        std::cout << "Available extension: " << props[i].extensionName << std::endl;
    }

    for (unsigned int i = 0; i < instance_extension_names.size(); i++) {
        std::cout << "Tried extension: " << instance_extension_names[i] << std::endl;
	} */

    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> availableLayers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, availableLayers.data());

    /* for (VkLayerProperties& vv : availableLayers) {
        std::cout << "Available layer: " << vv.layerName << std::endl;
    }

    for (unsigned int i = 0; i < instance_layer_names.size(); i++) {
        std::cout << "Tried layer: " << instance_layer_names[i] << std::endl;
	} */

    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("Wingine").setApplicationVersion(1)
      .setPEngineName("Wingine").setEngineVersion(1)
      .setApiVersion(VK_API_VERSION_1_0);

    vk::InstanceCreateInfo cInfo;
    cInfo.setPApplicationInfo(&appInfo).setEnabledExtensionCount(instance_extension_names.size())
      .setPpEnabledExtensionNames(instance_extension_names.data())
      .setEnabledLayerCount(instance_layer_names.size())
      .setPpEnabledLayerNames(instance_layer_names.size() ?
			      instance_layer_names.data() :
			      NULL);

    this->vulkan_instance = vk::createInstance(cInfo);

    
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    this->dispatcher.init(vkGetInstanceProcAddr);
    this->dispatcher.init(this->vulkan_instance);
    
#ifdef DEBUG
    
    vk::DebugReportCallbackCreateInfoEXT callbackInfo;
    callbackInfo.setFlags(vk::DebugReportFlagBitsEXT::eError |
			  vk::DebugReportFlagBitsEXT::eWarning |
			  vk::DebugReportFlagBitsEXT::ePerformanceWarning)
      .setPfnCallback(&_debugCallbackFun);

    this->debug_callback = this->vulkan_instance
      .createDebugReportCallbackEXT(callbackInfo, nullptr, this->dispatcher);
    
#endif // DEBUG

    this->window_width = width;
    this->window_height = height;
  }

  void Wingine::init_surface(winval_type_0 arg0, winval_type_1 arg1) {
#ifdef WIN32
    vk::Win32SurfaceCreateInfoKHR info;
    info.setHinstance(arg0).setHwnd(arg1);

    this->surface = this->vulkan_instance
      .createWin32SurfaceKHR(info, nullptr, this->dispatcher);
#else // WIN32
    vk::XlibSurfaceCreateInfoKHR info;
    info.setWindow(arg0).setDpy(arg1);

    this->surface = this->vulkan_instance
      .createXlibSurfaceKHR(info, nullptr, this->dispatcher);
#endif // WIN32
  }
  
  void Wingine::init_device() {
    std::vector<vk::PhysicalDevice> found_devices = this->vulkan_instance.enumeratePhysicalDevices();

    std::cout << "Number of devices: " << found_devices.size() << std::endl;

    bool found = false;

    for(vk::PhysicalDevice dev : found_devices) {
      vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceVulkan12Properties> props =
	dev.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceVulkan12Properties>(this->dispatcher);

      vk::PhysicalDeviceProperties2 props2 = props.get<vk::PhysicalDeviceProperties2>();
      // vk::PhysicalDeviceVulkan12Properties props12 = props.get<vk::PhysicalDeviceVulkan12Properties>();

      std::cout << "Device name: " << props2.properties.deviceName << std::endl;
      // std::cout << "maxTimelineSemaphoreValueDifference: " << props12.maxTimelineSemaphoreValueDifference << std::endl;
        this->graphics_queue_index = -1;
        this->present_queue_index = -1;
        this->compute_queue_index = -1;

        std::vector<vk::QueueFamilyProperties> qprops = dev.getQueueFamilyProperties();
        for(unsigned int i = 0; i <  qprops.size(); i++) {

	    vk::Bool32 supportsGraphics =
	        (qprops[i].queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlagBits {};
            if(supportsGraphics) {
	        this->graphics_queue_index = i;
	    }
	
	    vk::Bool32 supportsPresent = dev.getSurfaceSupportKHR(i, this->surface);
	    if(supportsPresent) {
	        this->present_queue_index = i;
	    }

	    bool supportsCompute =
	        (qprops[i].queueFlags & vk::QueueFlagBits::eCompute) != vk::QueueFlagBits {};
	    if(supportsCompute) {
	        this->compute_queue_index = i;
	    }
    }

    if(this->graphics_queue_index != -1 &&
	    this->present_queue_index != -1) {
	
	if(this->compute_queue_index == -1) {
	  _wlog_error("Chosen graphics device does not support compute kernels");
	}
	
	this->physical_device = dev;
	this->device_memory_props = dev.getMemoryProperties();
       
	found = true;
	break;
      }
    }

    if(!found) {
      _wlog_error("Could not find device supporting both presenting and graphics");
      std::exit(-1);
    }

    
    std::vector<const char*> device_extension_names;
    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    std::vector<vk::DeviceQueueCreateInfo> c_infos;
    c_infos.reserve(3);

    vk::DeviceQueueCreateInfo c_info;
    float queue_priorities[1] = {1.0f};
    c_info.setQueueCount(1).setPQueuePriorities(queue_priorities)
      .setQueueFamilyIndex(this->present_queue_index);
    c_infos.push_back(c_info);

    if (this->present_queue_index != this->graphics_queue_index) {
        c_info.setQueueFamilyIndex(this->graphics_queue_index);
        c_infos.push_back(c_info);
    }

    if (this->compute_queue_index >= 0 &&
	(this->compute_queue_index != this->present_queue_index &&
	 this->compute_queue_index != this->graphics_queue_index)) {
        c_info.setQueueFamilyIndex(this->compute_queue_index);
        c_infos.push_back(c_info);
    }

    vk::PhysicalDeviceFeatures feats = {};
    feats.setShaderClipDistance(VK_TRUE);

    vk::PhysicalDeviceVulkan12Features feats12;
    feats12.setTimelineSemaphore(VK_TRUE);
    
    vk::DeviceCreateInfo device_info;
    device_info.setQueueCreateInfoCount(c_infos.size())
      .setPQueueCreateInfos(c_infos.data())
      .setEnabledExtensionCount(device_extension_names.size())
      .setPpEnabledExtensionNames(device_extension_names.data())
      .setEnabledLayerCount(0)
      .setPpEnabledLayerNames(nullptr)
      .setPEnabledFeatures(&feats)
      .setPNext(&feats12); // The documentation says this is okay

    vk::PhysicalDeviceProperties phprops;
    this->physical_device.getProperties(&phprops);

    this->device = this->physical_device.createDevice(device_info);


    this->dispatcher.init(this->device);

    this->graphics_queue =
      this->device.getQueue(this->graphics_queue_index, 0);

    // If graphics and present queue indices are equal, make queues equal
    if(this->graphics_queue_index == this->present_queue_index) {
      this->present_queue = this->graphics_queue;
    } else {
      this->present_queue =
	this->device.getQueue(this->present_queue_index, 0);
    }

    if(this->compute_queue_index >= 0) {
      this->compute_queue =
	this->device.getQueue(this->compute_queue_index, 0);
    }
  }

  void Wingine::init_command_buffers() {
    vk::CommandPoolCreateInfo cpi;
    cpi.setQueueFamilyIndex(this->graphics_queue_index).
      setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    this->graphics_command_pool = this->device.createCommandPool(cpi);
    
    cpi.setQueueFamilyIndex(this->present_queue_index);
    this->present_command_pool = this->device.createCommandPool(cpi);

    vk::CommandBufferAllocateInfo cbi;
    cbi.setCommandPool(this->present_command_pool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount(1); // Premature optimization... etc.

    this->present_command.buffer = this->device.allocateCommandBuffers(cbi)[0];

    this->present_command.buffer
      .reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    vk::FenceCreateInfo fci;
    fci.setFlags(vk::FenceCreateFlagBits::eSignaled);
    
    this->present_command.fence =
      this->device.createFence(fci);

    
    this->general_purpose_command.fence =
      this->device.createFence(fci);

    cbi.setCommandPool(this->graphics_command_pool);
    this->general_purpose_command.buffer =
      this->device.allocateCommandBuffers(cbi)[0];
    
    if(this->compute_queue_index >= 0) {
      // Reuse CreateInfo
      cpi.setQueueFamilyIndex(this->compute_queue_index);
      this->compute_command_pool = this->device.createCommandPool(cpi);

      // Reuse AllocateInfo
      cbi.setCommandPool(this->compute_command_pool);
      this->compute_command.buffer = this->device.allocateCommandBuffers(cbi)[0];

      this->compute_command.buffer
	.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

      this->compute_command.fence =
	this->device.createFence(fci);
    }

    this->image_acquired_fence =
      this->device.createFence(fci);

    vk::SemaphoreCreateInfo sci;
    this->image_acquire_semaphore =
      this->device.createSemaphore(sci);
    this->finished_drawing_semaphore =
      this->device.createSemaphore(sci);
    
  }

  void Wingine::init_swapchain() {
    std::vector<vk::SurfaceFormatKHR> surfaceFormats =
      this->physical_device.getSurfaceFormatsKHR(this->surface);

    vk::ColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;
    
    if(surfaceFormats.size() == 1 &&
       surfaceFormats[0].format == vk::Format::eUndefined) {
      
      this->surface_format = vk::Format::eB8G8R8A8Unorm;
      
    } else {
      
      this->surface_format = surfaceFormats[0].format;
      
    }

    vk::SurfaceCapabilitiesKHR caps =
      this->physical_device.getSurfaceCapabilitiesKHR(this->surface);

    vk::Extent2D swapchainExtent;

    if(caps.currentExtent.width == 0xFFFFFFFF) {
      swapchainExtent.width =
	std::min(caps.maxImageExtent.width,
		 std::max(caps.minImageExtent.width,
			  this->window_width));
      swapchainExtent.height =
	std::min(caps.maxImageExtent.height,
		 std::max(caps.minImageExtent.height,
			  this->window_height));
      
    } else {
      swapchainExtent = caps.currentExtent;
      this->window_width = caps.currentExtent.width;
      this->window_height = caps.currentExtent.height;
    }

    uint32_t numSwaps =
      std::max(caps.minImageCount, (uint32_t)2);

    if(caps.maxImageCount != 0) {
      numSwaps =
	    std::min(caps.maxImageCount, numSwaps);
    }


    vk::SurfaceTransformFlagBitsKHR preTransform;
    if((caps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) ==
       vk::SurfaceTransformFlagBitsKHR::eIdentity) {
      preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
      preTransform = caps.currentTransform;
    }


    std::vector<vk::PresentModeKHR> presentModes =
      this->physical_device.getSurfacePresentModesKHR(this->surface);

    vk::PresentModeKHR swapchainPresentMode =
      vk::PresentModeKHR::eFifo;

    for(vk::PresentModeKHR mode : presentModes) {
      if(mode == vk::PresentModeKHR::eMailbox) {
	swapchainPresentMode = vk::PresentModeKHR::eMailbox;
      }
    }

    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
      vk::CompositeAlphaFlagBitsKHR::eOpaque;

    vk::CompositeAlphaFlagBitsKHR alphaFlags[4] = {
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
      vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
      vk::CompositeAlphaFlagBitsKHR::eInherit
    };

    for(vk::CompositeAlphaFlagBitsKHR bit : alphaFlags) {
      if((caps.supportedCompositeAlpha & bit) == bit) {
	compositeAlpha = bit;
	break;
      }
    }

    vk::SwapchainCreateInfoKHR sci;
    sci.setSurface(this->surface)
      .setMinImageCount(numSwaps)
      .setImageFormat(this->surface_format)
      .setImageExtent(swapchainExtent)
      .setPreTransform(preTransform)
      .setCompositeAlpha(compositeAlpha)
      .setImageArrayLayers(1)
      .setPresentMode(swapchainPresentMode)
      .setOldSwapchain(nullptr)
      .setClipped(true)
      .setImageColorSpace(colorSpace)
      // .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment |
		//      vk::ImageUsageFlagBits::eTransferSrc)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
      .setImageSharingMode(vk::SharingMode::eExclusive)
      .setQueueFamilyIndexCount(0)
      .setPQueueFamilyIndices(nullptr);

    uint32_t queue_indices[2] = {(uint32_t)this->graphics_queue_index,
				 (uint32_t)this->present_queue_index};
    if(this->graphics_queue_index != this->present_queue_index) {

      sci.setImageSharingMode(vk::SharingMode::eConcurrent)
	.setQueueFamilyIndexCount(2)
	.setPQueueFamilyIndices(queue_indices);
      
    }

    this->swapchain = this->device.createSwapchainKHR(sci);

    this->swapchain_images = this->device.getSwapchainImagesKHR(swapchain);
    
  }

  void Wingine::init_generic_render_pass() {
    this->register_compatible_render_pass(renColorDepth);
  }

  void Wingine::init_framebuffers() {
    for(unsigned int i = 0; i < this->swapchain_images.size(); i++) {
        vk::Image sim = this->swapchain_images[i];

	Framebuffer* framebuffer = new Framebuffer();;
      
      framebuffer->colorImage.image = sim;
      framebuffer->colorImage.width = this->window_width;
      framebuffer->colorImage.height = this->window_height;
      vk::MemoryAllocateInfo mai;

      this->cons_image_view(framebuffer->colorImage,
			    wImageViewColor);
      

      this->cons_image_image(framebuffer->depthImage,
			     this->window_width,
			     this->window_height,
			     vk::Format::eD32Sfloat,
			     vk::ImageUsageFlagBits::eDepthStencilAttachment |
			     vk::ImageUsageFlagBits::eTransferSrc,
			     vk::ImageTiling::eOptimal);
      this->cons_image_memory(framebuffer->depthImage,
			      vk::MemoryPropertyFlagBits::eDeviceLocal);
      this->cons_image_view(framebuffer->depthImage,
			    wImageViewDepth);

      vk::ImageView attachments[] = {
	framebuffer->colorImage.view,
	framebuffer->depthImage.view
      };
      
      vk::FramebufferCreateInfo finf;
      finf.setRenderPass(this->compatibleRenderPassMap[renColorDepth])
	.setAttachmentCount(2)
	.setPAttachments(attachments)
	.setWidth(this->window_width)
	.setHeight(this->window_height)
	.setLayers(1);
      
      framebuffer->framebuffer = this->device.createFramebuffer(finf);
      
      this->framebuffers.push_back(framebuffer);
    }
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

  void Wingine::waitForLastPresent() {
      this->device.waitForFences(1, &this->image_acquired_fence, true, UINT64_MAX);
  }

  void Wingine::stage_next_image(const std::initializer_list<SemaphoreChain*>& semaphores) {
    int num_semaphores = semaphores.size();

    this->waitForLastPresent();
    this->device.resetFences(1, &this->image_acquired_fence);
    
    this->device.acquireNextImageKHR(this->swapchain, UINT64_MAX,
				     num_semaphores ? this->image_acquire_semaphore : vk::Semaphore((VkSemaphore)(VK_NULL_HANDLE)),
				     image_acquired_fence,
				     &(this->current_swapchain_image));

    if(num_semaphores) {
      SemaphoreChain::semaphoreToChains(this, this->image_acquire_semaphore, std::begin(semaphores), num_semaphores);
    }
  }
  
  void Wingine::init_vulkan(int width, int height,
			    winval_type_0 arg0, winval_type_1 arg1, const char* str) {
    this->init_instance(width, height, str);
  
    this->init_surface(arg0, arg1);
  
    this->init_device();

    this->init_command_buffers();

    this->init_swapchain();

    this->init_generic_render_pass();
    
    this->init_framebuffers();

    this->init_descriptor_pool();

    this->init_pipeline_cache();

    this->stage_next_image({});
    this->waitForLastPresent(); // Ensure image is already acquired    
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
						 this->device_memory_props);
    image.memory = this->device.allocateMemory(mai);
    this->device.bindImageMemory(image.image, image.memory, 0); // 0 offset from memory start
  }
  
  void Wingine::cons_image_view(Image& image,
				ImageViewType type) {
    vk::ImageViewCreateInfo ivci;
    ivci.setImage(image.image)
      .setViewType(vk::ImageViewType::e2D)
      .setComponents({vk::ComponentSwizzle::eR,
	    vk::ComponentSwizzle::eG,
	    vk::ComponentSwizzle::eB,
	    vk::ComponentSwizzle::eA});
    
    switch(type) {
    case wImageViewColor:
      ivci.setFormat(vk::Format::eB8G8R8A8Unorm)
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

  Wingine::Wingine(int width, int height,
		   winval_type_0 arg0, winval_type_1 arg1, const char* str) {
    this->init_vulkan(width, height,
		      arg0, arg1, str);
  }

  
  Wingine::Wingine(Winval& win) {
#ifdef WIN32
    
    this->init_vulkan(win.getWidth(), win.getHeight(),
		      win.getInstance(), win.getHWND(), win.getTitle());
    
#else // WIN32
    
    this->init_vulkan(win.getWidth(), win.getHeight(),
		      win.getWindow(), win.getDisplay(), win.getTitle());
    
#endif // WIN32
  }

  vk::Device Wingine::getDevice() {
    return this->device;
  }

  vk::Queue Wingine::getGraphicsQueue() {
    return this->graphics_queue;
  }

  Command Wingine::getCommand() {
    return this->general_purpose_command;
  }

  vk::CommandPool Wingine::getPresentCommandPool() {
    return this->present_command_pool;
  }

  vk::CommandPool Wingine::getGraphicsCommandPool() {
      return this->graphics_command_pool;
  }

  vk::DescriptorPool Wingine::getDescriptorPool() {
    return this->descriptor_pool;
  }

  std::vector<Framebuffer*>&  Wingine::getFramebuffers() {
    return framebuffers;
  }

  int Wingine::getNumFramebuffers() {
    return framebuffers.size();
  }

  Framebuffer* Wingine::getCurrentFramebuffer() {
    return this->framebuffers[this->current_swapchain_image];
  }

  int Wingine::getCurrentFramebufferIndex() {
    return this->current_swapchain_image;
  }
  
  IndexBuffer* Wingine::createIndexBuffer(uint32_t numIndices) {
    return new IndexBuffer(*this, numIndices);
  }

  Shader* Wingine::createShader(uint64_t shader_bit,
				 std::vector<uint32_t>& spirv) {
    return new Shader(this->device,
		      shader_bit,
		      spirv);
  }

  
  Pipeline* Wingine::createPipeline(const std::vector<VertexAttribDesc>& descriptions,
				   const std::vector<std::vector<uint64_t> >& resourceSetLayout,
				   const std::vector<Shader*>& shaders,
				   bool depthOnly, int width, int height) {
    std::vector<ResourceSetLayout> rsl;
    for(unsigned int i = 0; i < resourceSetLayout.size(); i++) {
      rsl.push_back(this->resourceSetLayoutMap[resourceSetLayout[i]]);
    }

    return new Pipeline(*this,
			width < 0 ? this->window_width : width,
			height < 0 ? this->window_height : height,
			descriptions,
			rsl,
			shaders,
			depthOnly);
  }

  Framebuffer* Wingine::createFramebuffer(uint32_t width, uint32_t height,
					   bool depthOnly) {
    Framebuffer* framebuffer = new Framebuffer(*this,
					       width, height,
					       depthOnly);
    return framebuffer;
    
  }

  Texture* Wingine::createTexture(uint32_t width, uint32_t height, bool depth) {
    Texture* texture = new Texture(*this,
				    width, height, depth);
    return texture;
  }

  SemaphoreChain* Wingine::createSemaphoreChain() {
    SemaphoreChain* semaphore_chain = new SemaphoreChain(*this);
    return semaphore_chain;
  }

  StorageBuffer* Wingine::createStorageBuffer(uint32_t num_bytes, bool host_updatable) {
    return new StorageBuffer(*this,
			     num_bytes, host_updatable);
  }

  RenderFamily* Wingine::createRenderFamily(Pipeline* pipeline, bool clear, int num_framebuffers) {
    return new RenderFamily(*this,
			    pipeline, clear, num_framebuffers);
  }
  
  void Wingine::destroySwapchainImage(Image& image) {
    this->device.free(image.memory);
    this->device.destroy(image.view);
  }
  
  void Wingine::destroySwapchainFramebuffer(Framebuffer* framebuffer) {
    this->destroySwapchainImage(framebuffer->colorImage);
    this->destroy(framebuffer->depthImage);
    
    this->device.destroy(framebuffer->framebuffer);
  }

  void Wingine::destroy(RenderFamily* family) {
    for(int i = 0; i < family->num_buffers; i++) {
      this->device.waitForFences(1, &family->commands[i].fence, true, UINT64_MAX);
      this->device.destroy(family->commands[i].fence);

      this->device.freeCommandBuffers(this->graphics_command_pool,
				      1, &family->commands[i].buffer);

      if(family->render_passes[i] != this->compatibleRenderPassMap[family->pipeline->render_pass_type]) {
	this->device.destroy(family->render_passes[i]);
      }
    }
    
    delete family;
  }

  void Wingine::destroy(Shader* shader) {
    this->device.destroy(shader->shader_info.module);
    delete shader;
  }

  void Wingine::destroy(Texture* texture) {
    delete texture->image_info;

    this->device.destroy(texture->sampler);
    this->destroy(*(Image*)texture);

    this->device.destroy(texture->staging_image);
    this->device.free(texture->staging_memory);
    delete texture;
  }

  void Wingine::destroy(SemaphoreChain* semaphore_chain) {
    this->device.destroy(semaphore_chain->semaphore);

    delete semaphore_chain;
  }
  
  void Wingine::destroy(Pipeline* pipeline) {
    this->device.destroy(pipeline->layout);
    this->device.destroy(pipeline->pipeline);
    delete pipeline;
  }

  void Wingine::destroy(Buffer* buffer) {
    this->device.destroy(buffer->buffer);
    this->device.free(buffer->memory);

    if( !buffer->host_updatable ) {
      this->device.destroy(buffer->update_buffer);
      this->device.free(buffer->update_memory);
    }

    delete buffer;
  }

  void Wingine::destroy(Image& image) {
    this->device.destroy(image.image);
    this->device.free(image.memory);
    this->device.destroy(image.view);
  }
  
  void Wingine::destroy(Framebuffer* framebuffer) {
    this->destroy(framebuffer->colorImage);
    this->destroy(framebuffer->depthImage);

    this->device.destroy(framebuffer->framebuffer);
    
    delete framebuffer;
  }

  void Wingine::destroy(StorageBuffer* storagebuffer) {
    this->destroy(storagebuffer->buffer);
    delete storagebuffer->buffer_info;
    delete storagebuffer;
  }
  
  Wingine::~Wingine() {
    
    this->device.destroy(this->descriptor_pool);
    this->device.destroy(this->pipeline_cache);

    for(Framebuffer* fb : this->framebuffers) {
      this->destroySwapchainFramebuffer(fb);
      delete fb;
    }

    this->device.freeCommandBuffers(this->graphics_command_pool,
				    1, &this->general_purpose_command.buffer);
    this->device.freeCommandBuffers(this->present_command_pool,
				    1, &this->present_command.buffer);
    
    
    this->device.destroyCommandPool(this->graphics_command_pool);
    
    this->device.destroyCommandPool(this->present_command_pool);
    this->device.waitForFences(1, &this->present_command.fence, true, UINT64_MAX);
    this->device.destroyFence(this->present_command.fence);


    if(this->compute_queue_index >= 0) {
      this->device.freeCommandBuffers(this->compute_command_pool,
				      1, &this->compute_command.buffer);
      
      this->device.destroyCommandPool(this->compute_command_pool);
      this->device.waitForFences(1, &this->compute_command.fence, true, UINT64_MAX);
      this->device.destroyFence(this->compute_command.fence);
    }

    this->device.destroyFence(this->image_acquired_fence);

    this->device.waitForFences(1, &this->general_purpose_command.fence, true, UINT64_MAX);
    this->device.destroy(this->general_purpose_command.fence);

    this->device.destroy(this->swapchain);
    
    this->vulkan_instance.destroy(this->surface);
    
    for (auto it : this->resourceSetLayoutMap) {
      this->device.destroy(it.second.layout);
    }

    for(auto it : this->compatibleRenderPassMap) {
      this->device.destroy(it.second);
    }
    
    this->device.destroy();
    
    this->vulkan_instance.destroyDebugReportCallbackEXT(this->debug_callback,
						  nullptr, this->dispatcher);
    this->vulkan_instance.destroy();
  }
  
};
