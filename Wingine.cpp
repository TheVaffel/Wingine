#include "Wingine.hpp"

#ifdef WIN32

#define VK_USE_PLATFORM_WIN32_KHR
#include "external/vulkan_win32.h"

#include <algorithm>

#undef min
#undef max

#else // WIN32
#include <unistd.h>

#define VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan_xlib.h"

#endif // WIN32

#include <iostream>
#include "vulkan/vulkan.hpp"


#define DEBUG

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
 * Utility functions
 */

static void _wlog_error(std::string str) {
  std::cout << "[Wingine Error] " << str << std::endl;
}

static void _wassert(bool test, std::string str = "Error") {
  if(!test) {
    _wlog_error(str);
    exit(-1);
  }
}

static uint32_t _get_memory_type_index(uint32_t type_bits,
				       vk::MemoryPropertyFlags requirements_mask,
				       vk::PhysicalDeviceMemoryProperties& mem_props) {
  for(uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
    if ((type_bits & (1 << i))) { 
      if ((mem_props.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
	return i;
      }
    }
  }

  _wlog_error("Could not find appropriate memory type");
  exit(-1);
}

static vk::Format _get_format(wg::ComponentType type, int num_components) {
  _wassert(num_components > 0 && num_components <= 4,
	   "Number of components in vertex attribute can be at most 4");
  // At least for now, formats are a bit strict

  // VK_FORMAT_R32_SFLOAT = 100 etc.. are defined in vulkan_core.h
  
  switch(type) {
  case wg::tFloat32:
    return vk::Format(97 + num_components * 3);
  case wg::tFloat64:
    return vk::Format(109 + num_components * 3);
  case wg::tInt32:
    return vk::Format(96 + num_components * 3);
  case wg::tInt64:
    return vk::Format(95 + num_components * 3);
  }

  return vk::Format::eR32Sfloat;
}

/*
 * O boi here we go
 */

namespace wg {

  ResourceSetLayout::ResourceSetLayout(Wingine& wing, const std::vector<uint64_t>& flags) {
    std::vector<vk::DescriptorSetLayoutBinding> lbs(flags.size());

    this->wing = &wing;

    vk::Device device = this->wing->getDevice();
    
    int i = 0;
    for(uint64_t flag : flags) {
      lbs[i].setBinding(i)
	.setDescriptorCount(1)
	.setStageFlags(vk::ShaderStageFlagBits(flag >> 32))
	.setDescriptorType(vk::DescriptorType(flag & ((1LL << 32) - 1)));
      i++;
    }

    vk::DescriptorSetLayoutCreateInfo dlc;
    dlc.setBindingCount((uint32_t)lbs.size())
      .setPBindings(lbs.data());

    this->layout =
      device.createDescriptorSetLayout(dlc);
      
  }

  ResourceSetLayout::ResourceSetLayout() { }

  ResourceSet Wingine::createResourceSet(std::vector<uint64_t>& flags) {
    
    if(this->resourceSetLayoutMap.find(flags) ==
       this->resourceSetLayoutMap.end()) {

      this->resourceSetLayoutMap[flags] = ResourceSetLayout(*this, flags);
    }
    
    return ResourceSet(*this, this->resourceSetLayoutMap[flags].layout);
  }

  ResourceSet::ResourceSet(Wingine& wing, vk::DescriptorSetLayout layout) {
    vk::DescriptorSetAllocateInfo dsai;
    dsai.setDescriptorPool(wing.getDescriptorPool())
      .setDescriptorSetCount(1)
      .setPSetLayouts(&layout);

    vk::Device device = wing.getDevice();
    this->descriptor_set = device.allocateDescriptorSets(dsai)[0];
    this->device = device;
  }

  void ResourceSet::set(const std::vector<Resource*>& resources) {
    std::vector<vk::WriteDescriptorSet> writes(resources.size());

    for(unsigned int i = 0; i < resources.size(); i++) {
      writes[i].setDstSet(this->descriptor_set)
	.setDescriptorCount(1)
	.setDescriptorType(resources[i]->type)
	.setPBufferInfo(resources[i]->buffer_info)
	.setPImageInfo(resources[i]->image_info)
	.setDstArrayElement(0)
	.setDstBinding(i);
    }

    this->device.updateDescriptorSets(writes, {});
  }
  
  Shader::Shader(vk::Device& device, uint64_t shader_type, std::vector<uint32_t>& spirv) {
    
    vk::ShaderStageFlagBits stage_bit;
    switch(shader_type) {
    case shaVertex:
      stage_bit = vk::ShaderStageFlagBits::eVertex;
      break;
    case shaFragment:
      stage_bit = vk::ShaderStageFlagBits::eFragment;
      break;
    }

    this->shader_info.setStage(stage_bit)
      .setPName("main");

    vk::ShaderModuleCreateInfo smci;
    smci.setCodeSize(spirv.size() * sizeof(uint32_t))
      .setPCode(spirv.data());
    
    
    this->shader_info.module = device.createShaderModule(smci);
    
  }

  Pipeline::Pipeline(Wingine& wing,
		     int width, int height,
		     const std::vector<VertexAttribDesc>& descriptions,
		     const std::vector<ResourceSetLayout>& resourceSetLayouts,
		     const std::vector<Shader*>& shaders, bool depthOnly) {

    vk::Device device = wing.getDevice();
    
    int vertex_binding_count = 0;
    
    for(VertexAttribDesc desc : descriptions) {
      vertex_binding_count = std::max(desc.binding_num + 1,
				      (uint32_t)vertex_binding_count);
    }
    

    // Defaults okay sometimes
    vk::PipelineDynamicStateCreateInfo state_info;
    std::vector<vk::VertexInputBindingDescription> vi_bindings(vertex_binding_count);
    std::vector<vk::VertexInputAttributeDescription> vi_attribs(descriptions.size());
    vk::PipelineVertexInputStateCreateInfo vi;
    vk::PipelineInputAssemblyStateCreateInfo ia;
    vk::PipelineRasterizationStateCreateInfo rs;
    vk::PipelineColorBlendStateCreateInfo cb;
    vk::PipelineColorBlendAttachmentState att_state;
    vk::PipelineViewportStateCreateInfo vp;
    vk::PipelineDepthStencilStateCreateInfo ds;
    vk::PipelineMultisampleStateCreateInfo ms;
    vk::PipelineLayoutCreateInfo layoutCreateInfo;

    vk::GraphicsPipelineCreateInfo createInfo;

    for(int i = 0; i < vertex_binding_count; i++) {
      vi_bindings[i].setBinding(i)
	.setInputRate(vk::VertexInputRate::eVertex);
    }

    for(unsigned int i = 0; i < descriptions.size(); i++) {
      vi_bindings[descriptions[i].binding_num]
	.setStride(descriptions[i].stride_in_bytes); // Set stride of binding structure

      vi_attribs[i].setBinding(descriptions[i].binding_num)
	.setLocation(i)
	.setFormat(_get_format(descriptions[i].component_type,
			       descriptions[i].num_elements))
	.setOffset(descriptions[i].offset_in_bytes);
    }
    
    vk::Viewport viewport;
    viewport.setWidth((float)width)
      .setHeight((float)height)
      .setMinDepth(0.0f)
      .setMaxDepth(1.0f)
      .setX(0.f)
      .setY(0.f);

    vk::Rect2D scissor;
    scissor.setExtent({(uint32_t)width, (uint32_t)height})
      .setOffset({(uint32_t)0, (uint32_t)0});
    
    vi.setVertexBindingDescriptionCount(vertex_binding_count)
      .setPVertexBindingDescriptions(vi_bindings.data())
      .setVertexAttributeDescriptionCount(descriptions.size())
      .setPVertexAttributeDescriptions(vi_attribs.data());

    ia.setPrimitiveRestartEnable(false)
      .setTopology(vk::PrimitiveTopology::eTriangleList);

    rs.setPolygonMode(vk::PolygonMode::eFill)
      .setCullMode(vk::CullModeFlagBits::eNone)
      .setFrontFace(vk::FrontFace::eClockwise)
      .setDepthClampEnable(false)
      .setRasterizerDiscardEnable(false)
      .setDepthBiasEnable(false)
      .setDepthBiasConstantFactor(0)
      .setDepthBiasClamp(0)
      .setDepthBiasSlopeFactor(0)
      .setLineWidth(1.0f);

    att_state.setColorWriteMask(vk::ColorComponentFlags(0xf))
      .setBlendEnable(true)
      .setAlphaBlendOp(vk::BlendOp::eAdd)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
      .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
      .setSrcAlphaBlendFactor(vk::BlendFactor::eZero)
      .setDstAlphaBlendFactor(vk::BlendFactor::eOne);
    
    cb.setPAttachments(&att_state)
      .setAttachmentCount(1)
      .setLogicOpEnable(false)
      .setLogicOp(vk::LogicOp::eNoOp)
      .setBlendConstants({1.0f, 1.0f,
	    1.0f, 1.0f});

    vp.setViewportCount(1)
      .setScissorCount(1)
      .setPScissors(&scissor)
      .setPViewports(&viewport);

    ds.setDepthTestEnable(true)
      .setDepthWriteEnable(true)
      .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
      .setDepthBoundsTestEnable(false)
      .setBack({vk::StencilOp::eKeep,
	    vk::StencilOp::eKeep,
	    vk::StencilOp::eKeep,
	    vk::CompareOp::eAlways,
	    0, 0, 0})
      .setMinDepthBounds(0)
      .setMaxDepthBounds(0)
      .setStencilTestEnable(false);
    ds.setFront(ds.back);

    ms.setPSampleMask(nullptr)
      .setRasterizationSamples(vk::SampleCountFlagBits::e1)
      .setSampleShadingEnable(false)
      .setAlphaToCoverageEnable(false)
      .setAlphaToOneEnable(false)
      .setMinSampleShading(0.0f);

    
    std::vector<vk::DescriptorSetLayout> layouts(resourceSetLayouts.size());
    for(unsigned int i = 0; i < resourceSetLayouts.size(); i++) {
      layouts[i] = resourceSetLayouts[i].layout;
    }
    
    layoutCreateInfo.setPushConstantRangeCount(0)
      .setPPushConstantRanges(nullptr)
      .setSetLayoutCount(layouts.size())
      .setPSetLayouts(layouts.data());

    this->layout = device.createPipelineLayout(layoutCreateInfo);

    std::vector<vk::PipelineShaderStageCreateInfo> pssci(shaders.size());
    for(unsigned int i = 0; i < shaders.size(); i++) {
      pssci[i] = shaders[i]->shader_info;
    }

    
    RenderPassType rpt;
      
    if (depthOnly) {
      rpt = renDepth;
    } else {
      rpt = renColorDepth;
    }

    

    if(wing.compatibleRenderPassMap.find(rpt) == wing.compatibleRenderPassMap.end()) {
      wing.register_compatible_render_pass(rpt);
    }
    
    
    createInfo.setLayout(this->layout)
      .setBasePipelineHandle(nullptr)
      .setBasePipelineIndex(0)
      .setPVertexInputState(&vi)
      .setPInputAssemblyState(&ia)
      .setPRasterizationState(&rs)
      .setPColorBlendState(&cb)
      .setPTessellationState(nullptr)
      .setPMultisampleState(&ms)
      .setPDynamicState(&state_info)
      .setPViewportState(&vp)
      .setPDepthStencilState(&ds)
      .setPStages(pssci.data())
      .setStageCount(pssci.size())
      .setRenderPass(wing.compatibleRenderPassMap[rpt])
      .setSubpass(0);

    this->render_pass_type = rpt;
    
    this->pipeline = device.createGraphicsPipelines(wing.pipeline_cache,
						    {createInfo})[0];

    
  }

  _Framebuffer::_Framebuffer() { }
  
  _Framebuffer::_Framebuffer(Wingine& wing,
			     int width, int height,
			     bool depthOnly,
			     bool withoutReadyToDrawSemaphore) {

    std::vector<vk::ImageView> attachments;
    
    if (!depthOnly) {
      wing.cons_image_image(this->colorImage,
			    width, height,
			    vk::Format::eB8G8R8A8Unorm,
			    vk::ImageUsageFlagBits::eColorAttachment |
			    vk::ImageUsageFlagBits::eTransferSrc,
			    vk::ImageTiling::eOptimal);
      wing.cons_image_memory(this->colorImage,
			     vk::MemoryPropertyFlagBits::eDeviceLocal);
      wing.cons_image_view(this->colorImage,
			   wImageViewColor);

      attachments.push_back(this->colorImage.view);
    }

    
    wing.cons_image_image(this->depthImage,
			  width, height,
			  vk::Format::eD32Sfloat,
			  vk::ImageUsageFlagBits::eDepthStencilAttachment |
			  vk::ImageUsageFlagBits::eTransferSrc,
			  vk::ImageTiling::eOptimal);
			  
    wing.cons_image_memory(this->depthImage,
			   vk::MemoryPropertyFlagBits::eDeviceLocal);
    wing.cons_image_view(this->depthImage,
			 wImageViewDepth);

    attachments.push_back(this->depthImage.view);

    RenderPassType render_pass_type =
      depthOnly ?
      renDepth :
      renColorDepth;

    if(wing.compatibleRenderPassMap.find(render_pass_type) ==
       wing.compatibleRenderPassMap.end()) {
      wing.register_compatible_render_pass(render_pass_type);
    }
    
    vk::FramebufferCreateInfo finf;
    finf.setRenderPass(wing.compatibleRenderPassMap[render_pass_type])
      .setAttachmentCount(attachments.size())
      .setPAttachments(attachments.data())
      .setWidth(width)
      .setHeight(height)
      .setLayers(1);

    vk::Device device = wing.getDevice();

    vk::SemaphoreCreateInfo sci;
    
    if (!withoutReadyToDrawSemaphore) {
      this->ready_for_draw_semaphore = new vk::Semaphore();
      
      *this->ready_for_draw_semaphore = wing.device.createSemaphore(sci);
    } else {
      this->ready_for_draw_semaphore = nullptr;
    }

    this->has_been_drawn_semaphore = new vk::Semaphore();

    *this->has_been_drawn_semaphore = wing.device.createSemaphore(sci);
    
    this->framebuffer = device.createFramebuffer(finf);
  }

  _Texture::_Texture(Wingine& wing,
		     uint32_t width, uint32_t height,
		     bool depth) :
    Resource(vk::DescriptorType::eCombinedImageSampler) {
    
    this->wing = &wing;
    this->aspect = depth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    vk::Device device = wing.getDevice();
    
    wing.cons_image_image(*this,
			  width, height,
			  depth ? vk::Format::eD32Sfloat : vk::Format::eB8G8R8A8Unorm,
			  vk::ImageUsageFlagBits::eSampled |
			  vk::ImageUsageFlagBits::eTransferDst,
			  vk::ImageTiling::eOptimal);
    wing.cons_image_memory(*this,
			   vk::MemoryPropertyFlagBits::eDeviceLocal);
    wing.cons_image_view(*this,
			 depth ? wImageViewDepth : wImageViewColor);


    this->width = width;
    this->height = height;

    if (!depth) {
        Image pseudo;
        wing.cons_image_image(pseudo,
            width, height,
            depth ? vk::Format::eD32Sfloat : vk::Format::eB8G8R8A8Unorm,
            vk::ImageUsageFlagBits::eTransferSrc,
            vk::ImageTiling::eLinear,
            vk::ImageLayout::ePreinitialized);
        wing.cons_image_memory(pseudo,
            vk::MemoryPropertyFlagBits::eHostCoherent |
            vk::MemoryPropertyFlagBits::eHostVisible);

        this->staging_image = pseudo.image;
        this->staging_memory = pseudo.memory;

        this->staging_memory_memreq = device.getImageMemoryRequirements(this->staging_image);

        this->current_staging_layout = vk::ImageLayout::eUndefined;



        vk::ImageSubresource subres;
        subres.setMipLevel(0)
            .setArrayLayer(0)
            .setAspectMask(this->aspect);

        vk::SubresourceLayout lay = device.getImageSubresourceLayout(this->staging_image, subres);

        this->stride_in_bytes = lay.rowPitch;
    }
      
    vk::SamplerCreateInfo sci;
    sci.setMagFilter(vk::Filter::eLinear)
      .setMinFilter(vk::Filter::eLinear)
      .setMipmapMode(vk::SamplerMipmapMode::eNearest)
      .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
      .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
      .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
      .setMipLodBias(0.0f)
      .setAnisotropyEnable(false)
      .setMaxAnisotropy(1)
      .setCompareOp(vk::CompareOp::eLess)
      .setMinLod(0.0f)
      .setMaxLod(1.0f)
      .setCompareEnable(false)
      .setBorderColor(vk::BorderColor::eFloatOpaqueWhite);

    this->sampler = device.createSampler(sci);

    
    this->image_info = new vk::DescriptorImageInfo();
    this->image_info->setSampler(this->sampler)
      .setImageView(this->view)
      .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
  }

  // Returns stride in bytes
  uint32_t _Texture::getStride() {
    return this->stride_in_bytes;
  }
   
  void _Texture::set(unsigned char* pixels, bool fixed_stride) {
    void* mapped_memory;
    uint32_t mem_size = this->staging_memory_memreq.size;

    vk::Device device = this->wing->getDevice();
    device.mapMemory(this->staging_memory, 0, mem_size, {}, &mapped_memory);

    if (fixed_stride) {
      memcpy(mapped_memory, pixels, mem_size);
    } else {
      unsigned char* curr_mapped = (unsigned char*)mapped_memory;
      unsigned char* curr_pixels = pixels;
      for (unsigned int i = 0; i < this->height; i++) {
	memcpy(curr_mapped, curr_pixels, this->width * 4);
	curr_mapped += this->stride_in_bytes;
	curr_pixels += this->width * 4;
      }
    }

    device.unmapMemory(this->staging_memory);

    this->wing->copy_image(this->width, this->height, this->staging_image,
			  this->current_staging_layout, vk::ImageLayout::eGeneral,
			  this->width, this->height, this->image,
			  this->current_layout, vk::ImageLayout::eShaderReadOnlyOptimal,
			  this->aspect);

    this->current_staging_layout = vk::ImageLayout::eGeneral;
    this->current_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
  }

  void _Texture::set(_Framebuffer* framebuffer) {
    bool depth = this->aspect == vk::ImageAspectFlagBits::eDepth;
    
    this->wing->copy_image(depth ? framebuffer->depthImage.width
			   : framebuffer->colorImage.width,
			   depth ? framebuffer->depthImage.height
			   : framebuffer->colorImage.height,
			   depth ? framebuffer->depthImage.image
			   : framebuffer->colorImage.image,
			   depth ? framebuffer->depthImage.current_layout
			   : framebuffer->colorImage.current_layout,
               depth ? vk::ImageLayout::eDepthStencilAttachmentOptimal 
               : vk::ImageLayout::eColorAttachmentOptimal,
			   this->width, this->height,
			   this->image, this->current_layout,
			   vk::ImageLayout::eShaderReadOnlyOptimal,
			   this->aspect,
			   framebuffer->has_been_drawn_semaphore);

    this->current_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
    
    if (depth) {
        framebuffer->depthImage.current_layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }
    else {
        framebuffer->colorImage.current_layout = vk::ImageLayout::eColorAttachmentOptimal;
    }
			   
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
			   vk::Semaphore *wait_semaphore) {
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

    vk::PipelineStageFlags last_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;	
    
    vk::SubmitInfo si;
    si.setCommandBufferCount(1)
      .setPCommandBuffers(&general_purpose_command.buffer)
      .setPWaitDstStageMask(&last_stage);
    
    if (wait_semaphore != nullptr) {
      si.setWaitSemaphoreCount(1)
	.setPWaitSemaphores(wait_semaphore);
    } else {
      si.setWaitSemaphoreCount(0);
    }
    
    this->graphics_queue.submit(1, &si, general_purpose_command.fence);

    // If we don't wait for it to finish, we cannot guarantee that it is actually ready for use
    this->device.waitForFences(1, &general_purpose_command.fence, true, (uint64_t)1e9);
    
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
  
  RenderFamily::RenderFamily(Wingine& wing,
			     Pipeline& pipeline,
			     bool clear) :
    wing(&wing), pipeline(&pipeline) {

    this->clears = clear;

    if(!clear) {
      this->render_pass = wing.compatibleRenderPassMap[pipeline.render_pass_type];
    } else {
      this->render_pass = wing.create_render_pass(pipeline.render_pass_type,
						  clear);
    }

    
    
    vk::CommandPool pool = wing.getGraphicsCommandPool();
    vk::Device device = wing.getDevice();
    
    vk::CommandBufferAllocateInfo cbi;
    cbi.setCommandPool(pool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount(1); // Premature optimization... etc.

    command.buffer = device.allocateCommandBuffers(cbi)[0];

    command.buffer
      .reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    vk::FenceCreateInfo fci;
    fci.setFlags(vk::FenceCreateFlagBits::eSignaled);
    
    command.fence =
      device.createFence(fci);
  }
  
  Buffer::Buffer(Wingine& wing,
		 vk::BufferUsageFlags usage,
		 uint32_t size,
		 bool host_updatable) {
    this->wing = &wing;
    
    vk::Device device = wing.getDevice();
    
    this->host_updatable = host_updatable;
    
    vk::BufferCreateInfo bci;
    bci.setSize(size)
      .setSharingMode(vk::SharingMode::eExclusive)
      .setUsage(usage |
		vk::BufferUsageFlagBits::eTransferDst);
    
    this->buffer = device.createBuffer(bci);

    vk::MemoryRequirements memReqs =
      device.getBufferMemoryRequirements(this->buffer);

    vk::MemoryAllocateInfo mai;
    mai.setAllocationSize(memReqs.size);

    if (host_updatable) {
      mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
						    vk::MemoryPropertyFlagBits::eHostCoherent |
						    vk::MemoryPropertyFlagBits::eHostVisible,
						    wing.device_memory_props));

      this->memory = device.allocateMemory(mai);
    } else {
      mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
						    vk::MemoryPropertyFlagBits::eDeviceLocal,
						    wing.device_memory_props));
      
      this->memory = device.allocateMemory(mai);
      
      bci.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
      
      this->update_buffer = device.createBuffer(bci);

      memReqs = device.getBufferMemoryRequirements(this->update_buffer);

      mai.setMemoryTypeIndex(_get_memory_type_index(memReqs.memoryTypeBits,
						    vk::MemoryPropertyFlagBits::eHostCoherent |
						    vk::MemoryPropertyFlagBits::eHostVisible,
						    wing.device_memory_props));
      this->update_memory = device.allocateMemory(mai);

      device.bindBufferMemory(this->update_buffer, this->update_memory, 0);
    }

    
    device.bindBufferMemory(this->buffer, this->memory, 0); // Buffer, memory, memoryOffset
  }

  void Buffer::set(void* data, uint32_t sizeInBytes, uint32_t offsetInBytes) {
    void* mapped;
    vk::Device device = this->wing->getDevice();
    
    if (this->host_updatable) {
      device.mapMemory(this->memory, offsetInBytes, sizeInBytes, {}, &mapped);

      memcpy(mapped, data, sizeInBytes);

      device.unmapMemory(this->memory);
    } else {      

      vk::Device device = this->wing->getDevice();
      Command command = this->wing->getCommand();
      vk::Queue queue = this->wing->getGraphicsQueue();
    
      
      device.mapMemory(this->update_memory, offsetInBytes, sizeInBytes, {}, &mapped);

      memcpy(mapped, data, sizeInBytes);

      device.unmapMemory(this->update_memory);

      vk::BufferCopy bc;
      bc.setSrcOffset(offsetInBytes)
	.setDstOffset(offsetInBytes)
	.setSize(sizeInBytes);
      
      vk::CommandBufferBeginInfo cbbi;

      command.buffer.begin(cbbi);

      command.buffer.copyBuffer(this->update_buffer,
				this->buffer,
				1, &bc);
      
      command.buffer.end();

      vk::SubmitInfo si;
      si.setCommandBufferCount(1)
	.setPCommandBuffers(&command.buffer);

      device.waitForFences(1, &command.fence, VK_TRUE,
			   (uint64_t)1e9);
      device.resetFences(1, &command.fence);
      
      queue.submit(1, &si, command.fence);

      device.waitForFences(1, &command.fence, VK_TRUE,
			   (uint64_t)1e9);
    }
  }

  IndexBuffer::IndexBuffer(Wingine& wing,
			   int numIndices,
			   bool host_updatable) :
    Buffer(wing,
	   vk::BufferUsageFlagBits::eIndexBuffer,
	   numIndices * sizeof(uint32_t),
	   host_updatable) {
    this->num_indices = numIndices;
  }

  void IndexBuffer::set(uint32_t* indices,
			uint32_t num,
			uint32_t offsetElements) {
    Buffer::set((void*)indices,
		num * sizeof(uint32_t),
		offsetElements * sizeof(uint32_t));
  }

  
  _VertexBuffer::_VertexBuffer(Wingine& wing,
			       vk::BufferUsageFlagBits bit,
			       int size,
			       bool host_updatable) :
    Buffer(wing,
	   bit,
	   size,
	   host_updatable) { }

  RenderObject::RenderObject(const std::vector<_VertexBuffer*>& buffers,
			     IndexBuffer nindexBuffer) :
    vertexBuffers(buffers), indexBuffer(nindexBuffer) { }

  Resource::Resource(vk::DescriptorType tp) :
    type(tp) {
    this->image_info = nullptr;
    this->buffer_info = nullptr;
  }

  void RenderFamily::startRecording(_Framebuffer* framebuffer) {

    if (framebuffer == nullptr) {
      framebuffer = wing->getCurrentFramebuffer();
    }
    
    vk::CommandBufferBeginInfo begin;
    
    vk::Rect2D renderRect;
    renderRect.setOffset({0, 0})
      .setExtent({framebuffer->depthImage.width, framebuffer->depthImage.height});
    
    vk::RenderPassBeginInfo rpb;
    rpb.setRenderPass(this->render_pass)
      .setClearValueCount(0)
      .setPClearValues(nullptr)
      .setFramebuffer(framebuffer->framebuffer)
      .setRenderArea(renderRect);

    this->current_framebuffer = framebuffer;
    
    // Size is number of attachments
    std::vector<vk::ClearValue> clear_values;

    if(this->clears) {
      switch (this->pipeline->render_pass_type) {
      case renDepth:
	clear_values.resize(1);
	clear_values[0].depthStencil.depth = 1.0f;
	clear_values[0].depthStencil.stencil = 0.0f;
	break;
      case renColorDepth:
	clear_values.resize(2);
	clear_values[0].color.setFloat32({0.3f, 0.3f,
	      0.3f, 1.0f});
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0.0f;
      }

      rpb.setClearValueCount(clear_values.size())
	.setPClearValues(clear_values.data());
    }
    
    vk::Device device = this->wing->getDevice();
    
    device.waitForFences(1, &this->command.fence, true, (uint64_t)(1000000));

    this->command.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    
    this->command.buffer.begin(begin);

    this->command.buffer.beginRenderPass(rpb, vk::SubpassContents::eInline);
    
    this->command.buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->pipeline->pipeline);
  }

  void RenderFamily::recordDraw(RenderObject& obj,
				std::vector<ResourceSet> sets) {
    std::vector<vk::DescriptorSet> d_sets(sets.size());
    for(unsigned int i = 0; i < sets.size(); i++) {
      d_sets[i] = sets[i].descriptor_set;
    }

    if (sets.size()) {
      this->command.buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
					      this->pipeline->layout,
					      0, d_sets.size(),
					      d_sets.data(),
					      0, nullptr);
    }

    std::vector<vk::Buffer> vertexBuffers(obj.vertexBuffers.size());
    std::vector<vk::DeviceSize> offsets(obj.vertexBuffers.size());

    for(unsigned int i = 0; i < vertexBuffers.size(); i++) {
      vertexBuffers[i] = obj.vertexBuffers[i]->buffer;
      offsets[i] = 0;
    }

    this->command.buffer.bindVertexBuffers(0, vertexBuffers.size(),
					   vertexBuffers.data(),
					   offsets.data());
    this->command.buffer.bindIndexBuffer(obj.indexBuffer.buffer,
					 0, vk::IndexType::eUint32);

    this->command.buffer.drawIndexed(obj.indexBuffer.num_indices, 1,
				     0, 0, 0);
  }

  void RenderFamily::endRecording() {
    this->command.buffer.endRenderPass();
    this->command.buffer.end();

    this->submit_command();
  }

  void RenderFamily::submit_command() {
    vk::Device device = this->wing->getDevice();
    vk::Queue queue = this->wing->getGraphicsQueue();

    vk::PipelineStageFlags last_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo si;
    si.setPWaitDstStageMask(&last_stage)
      .setCommandBufferCount(1)
      .setPCommandBuffers(&this->command.buffer)
      .setSignalSemaphoreCount(1)
      .setPSignalSemaphores(this->current_framebuffer->has_been_drawn_semaphore);

    if (this->current_framebuffer->ready_for_draw_semaphore != nullptr) {
      si.setWaitSemaphoreCount(1)
	.setPWaitSemaphores(this->current_framebuffer->ready_for_draw_semaphore);
    } else {
      si.setWaitSemaphoreCount(0);
    }
    
    device.resetFences(1, &this->command.fence);
    queue.submit(1, &si, this->command.fence);
    
    device.waitForFences(1, &this->command.fence, true, (uint64_t)1e9);
  }

  void Wingine::present() {

    vk::PresentInfoKHR presentInfo;

    presentInfo.setSwapchainCount(1)
      .setPSwapchains(&this->swapchain)
      .setPImageIndices(&this->current_swapchain_image)
      .setWaitSemaphoreCount(1)
      .setPWaitSemaphores(&this->image_drawn_semaphore)
      .setPResults(nullptr);

    
    this->present_queue.presentKHR(presentInfo);

    this->stage_next_image();
  }
  
  const vk::Image& Image::getImage() const {
    return this->image;
  }

  const vk::DeviceMemory& Image::getMemory() const {
    return this->memory;
  }

  const vk::ImageView& Image::getView() const {
    return this->view;
  }

  const Image& _Framebuffer::getColorImage() const {
    return this->colorImage;
  }

  const Image& _Framebuffer::getDepthImage() const {
    return this->depthImage;
  }

  const vk::Framebuffer& _Framebuffer::getFramebuffer() const {
    return this->framebuffer;
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
    for (unsigned int i = 0; i < extension_count; i++) {
        std::cout << "Available extension: " << props[i].extensionName << std::endl;
    }

    for (unsigned int i = 0; i < instance_extension_names.size(); i++) {
        std::cout << "Tried extension: " << instance_extension_names[i] << std::endl;
    }

    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> availableLayers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, availableLayers.data());
    for (VkLayerProperties& vv : availableLayers) {
        std::cout << "Available layer: " << vv.layerName << std::endl;
    }

    for (unsigned int i = 0; i < instance_layer_names.size(); i++) {
        std::cout << "Tried layer: " << instance_layer_names[i] << std::endl;
    }

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

  void Wingine::init_surface(_win_arg_type_0 arg0, _win_arg_type_1 arg1) {
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
        vk::PhysicalDeviceProperties props = dev.getProperties();
        std::cout << "Device name: " << props.deviceName << std::endl;


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

    vk::DeviceCreateInfo device_info;
    device_info.setQueueCreateInfoCount(c_infos.size())
      .setPQueueCreateInfos(c_infos.data())
      .setEnabledExtensionCount(device_extension_names.size())
      .setPpEnabledExtensionNames(device_extension_names.data())
      .setEnabledLayerCount(0)
      .setPpEnabledLayerNames(nullptr)
      .setPEnabledFeatures(&feats);

    vk::PhysicalDeviceProperties phprops;
    this->physical_device.getProperties(&phprops);

    std::cout << "Phys props: " << phprops.limits.maxImageArrayLayers << " " <<
        phprops.limits.maxImageDimension2D << std::endl;

    this->device = this->physical_device.createDevice(device_info);

    // this->device

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
    cpi.setQueueFamilyIndex(this->present_queue_index).
      setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    this->present_command_pool = this->device.createCommandPool(cpi);
    
    cpi.setQueueFamilyIndex(this->graphics_queue_index);
    this->graphics_command_pool = this->device.createCommandPool(cpi);

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

    vk::FenceCreateInfo fci;
    fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

    vk::SemaphoreCreateInfo sci2;
    this->image_acquired_semaphore = this->device.createSemaphore(sci2);
    this->image_drawn_semaphore = this->device.createSemaphore(sci2);
    
  }

  void Wingine::init_generic_render_pass() {
    this->register_compatible_render_pass(renColorDepth);
  }

  void Wingine::init_framebuffers() {
    for(unsigned int i = 0; i < this->swapchain_images.size(); i++) {
    // for(vk::Image sim : this->swapchain_images) {
        vk::Image sim = this->swapchain_images[i];

      _Framebuffer framebuffer;
      
      framebuffer.colorImage.image = sim;
      framebuffer.colorImage.width = this->window_width;
      framebuffer.colorImage.height = this->window_height;
      vk::MemoryAllocateInfo mai;

      this->cons_image_view(framebuffer.colorImage,
			    wImageViewColor);
      

      this->cons_image_image(framebuffer.depthImage,
			     this->window_width,
			     this->window_height,
			     vk::Format::eD32Sfloat,
			     vk::ImageUsageFlagBits::eDepthStencilAttachment |
			     vk::ImageUsageFlagBits::eTransferSrc,
			     vk::ImageTiling::eOptimal);
      this->cons_image_memory(framebuffer.depthImage,
			      vk::MemoryPropertyFlagBits::eDeviceLocal);
      this->cons_image_view(framebuffer.depthImage,
			    wImageViewDepth);

      vk::ImageView attachments[] = {
	framebuffer.colorImage.view,
	framebuffer.depthImage.view
      };
      
      vk::FramebufferCreateInfo finf;
      finf.setRenderPass(this->compatibleRenderPassMap[renColorDepth])
	.setAttachmentCount(2)
	.setPAttachments(attachments)
	.setWidth(this->window_width)
	.setHeight(this->window_height)
	.setLayers(1);
      
      framebuffer.framebuffer = this->device.createFramebuffer(finf);

      framebuffer.ready_for_draw_semaphore = &this->image_acquired_semaphore;
      framebuffer.has_been_drawn_semaphore = &this->image_drawn_semaphore;
      
      this->framebuffers.push_back(framebuffer);
    }
  }

  void Wingine::init_descriptor_pool() {
    const int max_num_descriptors = 16; // Use as placeholder - refactor this part if necessary
    
    std::vector<vk::DescriptorPoolSize> sizes(3);
    sizes[0].setType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(max_num_descriptors);

    sizes[1].setType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(max_num_descriptors);

    sizes[2].setType(vk::DescriptorType::eStorageImage)
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

  void Wingine::stage_next_image() {
    
    this->device.acquireNextImageKHR(this->swapchain, UINT64_MAX,
				     this->image_acquired_semaphore, nullptr,
				     &(this->current_swapchain_image));

  }
  
  void Wingine::init_vulkan(int width, int height,
			    _win_arg_type_0 arg0, _win_arg_type_1 arg1, const char* str) {
    this->init_instance(width, height, str);
  
    this->init_surface(arg0, arg1);
  
    this->init_device();

    this->init_command_buffers();

    this->init_swapchain();

    this->init_generic_render_pass();
    
    this->init_framebuffers();

    this->init_descriptor_pool();

    this->init_pipeline_cache();

    this->stage_next_image();
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
		   _win_arg_type_0 arg0, _win_arg_type_1 arg1, const char* str) {
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

  _Framebuffer* Wingine::getCurrentFramebuffer() {
    return &this->framebuffers[this->current_swapchain_image];
  }
  
  IndexBuffer Wingine::createIndexBuffer(uint32_t numIndices) {
    return IndexBuffer(*this, numIndices);
  }

  Shader Wingine::createShader(uint64_t shader_bit,
				 std::vector<uint32_t>& spirv) {
    return Shader(this->device,
		  shader_bit,
		  spirv);
  }

  
  Pipeline Wingine::createPipeline(const std::vector<VertexAttribDesc>& descriptions,
				   const std::vector<std::vector<uint64_t>* >& resourceSetLayout,
				   const std::vector<Shader*>& shaders,
				   bool depthOnly, int width, int height) {
    std::vector<ResourceSetLayout> rsl;
    for(unsigned int i = 0; i < resourceSetLayout.size(); i++) {
      rsl.push_back(this->resourceSetLayoutMap[*(resourceSetLayout[i])]);
    }

    return Pipeline(*this,
		    width < 0 ? this->window_width : width,
		    height < 0 ? this->window_height : height,
		    descriptions,
		    rsl,
		    shaders,
		    depthOnly);
  }

  _Framebuffer* Wingine::createFramebuffer(uint32_t width, uint32_t height,
					   bool depthOnly, bool withoutSemaphore) {
    _Framebuffer* framebuffer = new _Framebuffer(*this,
						 width, height,
						 depthOnly, withoutSemaphore);
    return framebuffer;
    
  }

  _Texture* Wingine::createTexture(uint32_t width, uint32_t height, bool depth) {
    _Texture* texture = new _Texture(*this,
				     width, height, depth);
    return texture;
  }

  RenderFamily Wingine::createRenderFamily(Pipeline& pipeline, bool clear) {
    return RenderFamily(*this,
			pipeline, clear);
  }
  
  void Wingine::destroySwapchainImage(Image& image) {
    this->device.free(image.memory);
    this->device.destroy(image.view);
  }
  
  void Wingine::destroySwapchainFramebuffer(_Framebuffer* framebuffer) {
    this->destroySwapchainImage(framebuffer->colorImage);
    this->destroy(framebuffer->depthImage);
    
    this->device.destroy(framebuffer->framebuffer);
  }

  void Wingine::destroy(RenderFamily& family) {
    this->device.waitForFences(1, &family.command.fence, true, UINT64_MAX);
    this->device.destroy(family.command.fence);

    if(family.render_pass != this->compatibleRenderPassMap[family.pipeline->render_pass_type]) {
      this->device.destroy(family.render_pass);
    }
  }

  void Wingine::destroy(Shader& shader) {
    this->device.destroy(shader.shader_info.module);
  }

  void Wingine::destroy(_Texture* texture) {
    delete texture->image_info;

    this->device.destroy(texture->sampler);
    this->destroy(*(Image*)texture);

    this->device.destroy(texture->staging_image);
    this->device.free(texture->staging_memory);
    delete texture;
  }
  
  void Wingine::destroy(ResourceSet& resourceSet) { }

  void Wingine::destroy(Pipeline& pipeline) {
    this->device.destroy(pipeline.layout);
    this->device.destroy(pipeline.pipeline);
  }

  void Wingine::destroy(Buffer& buffer) {
    this->device.destroy(buffer.buffer);
    this->device.free(buffer.memory);

    if( !buffer.host_updatable ) {
      this->device.destroy(buffer.update_buffer);
      this->device.free(buffer.update_memory);
    }
  }

  void Wingine::destroy(Image& image) {
    this->device.destroy(image.image);
    this->device.free(image.memory);
    this->device.destroy(image.view);
  }
  
  void Wingine::destroy(_Framebuffer* framebuffer) {
    this->destroy(framebuffer->colorImage);
    this->destroy(framebuffer->depthImage);

    this->device.destroy(framebuffer->framebuffer);
    this->device.destroy(*framebuffer->has_been_drawn_semaphore);

    if(framebuffer->ready_for_draw_semaphore != nullptr) {
          this->device.destroy(*framebuffer->ready_for_draw_semaphore);
	  delete framebuffer->ready_for_draw_semaphore;    
    }
    
    delete framebuffer->has_been_drawn_semaphore;
    
    delete framebuffer;
  }
  
  Wingine::~Wingine() {
    
    this->device.destroy(this->descriptor_pool);
    this->device.destroy(this->pipeline_cache);

    for(_Framebuffer& fb : this->framebuffers) {
      this->destroySwapchainFramebuffer(&fb);
    }
    
    this->device.destroyCommandPool(this->present_command_pool);
    this->device.waitForFences(1, &this->present_command.fence, true, UINT64_MAX);
    this->device.destroyFence(this->present_command.fence);

    this->device.destroyCommandPool(this->graphics_command_pool);

    if(this->compute_queue_index >= 0) {
      this->device.destroyCommandPool(this->compute_command_pool);
      this->device.waitForFences(1, &this->compute_command.fence, true, UINT64_MAX);
      this->device.destroyFence(this->compute_command.fence);
    }

    this->device.waitForFences(1, &this->general_purpose_command.fence, true, UINT64_MAX);
    this->device.destroy(this->general_purpose_command.fence);

    this->device.destroy(this->image_acquired_semaphore);
    this->device.destroy(this->image_drawn_semaphore);
    
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
