#include "resource.hpp"

#include "Wingine.hpp"

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

  Resource::Resource(vk::DescriptorType tp) :
    type(tp) {
    this->image_info = nullptr;
    this->buffer_info = nullptr;
  }

  /*
   * Texture - represents resource images with a sampler
   */

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
};