#include "framebuffer.hpp"

#include "Wingine.hpp"

namespace wg {
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

  
  const vk::Framebuffer& _Framebuffer::getFramebuffer() const {
    return this->framebuffer;
  }
};
