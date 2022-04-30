#include "./resource.hpp"
#include "./Wingine.hpp"

#include "./util/log.hpp"

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

    ResourceSet* Wingine::createResourceSet(const std::vector<uint64_t>& flags) {

        this->ensure_resource_set_layout_exists(flags);

        return new ResourceSet(*this, this->resourceSetLayoutMap[flags].layout);
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

    vk::DescriptorSet ResourceSet::getDescriptorSet() const {
        return this->descriptor_set;
    }

    Resource::Resource(vk::DescriptorType tp) :
        type(tp) {
        this->image_info = nullptr;
        this->buffer_info = nullptr;
    }


    /*
     * ResourceImage - represents resource images (storage images)
     */

    ResourceImage::ResourceImage(Wingine& wing,
                                 uint32_t width, uint32_t height) :
        Resource(vk::DescriptorType::eStorageImage) {

        vk::Format rgb_format = vk::Format::eR8G8B8A8Unorm;

        Image::constructImage(wing, *this,
                              width, height,
                              rgb_format,
                              vk::ImageUsageFlagBits::eTransferSrc |
                              vk::ImageUsageFlagBits::eStorage,
                              vk::ImageTiling::eOptimal,
                              vk::MemoryPropertyFlagBits::eDeviceLocal);

        vk::CommandBufferBeginInfo bg;
        _wassert_result(wing.device.waitForFences(1, &wing.getGeneralCommand().fence,
                                                  true, (uint64_t)1e9),
                        "wait for general purpose command in ResourceImage construction to finish");

        _wassert_result(wing.device.resetFences(1, &wing.getGeneralCommand().fence),
                        "reset fence in producing resource image");

        wing.getGeneralCommand().buffer.begin(bg);

        wing.cmd_set_layout(wing.getGeneralCommand().buffer, this->image,
                            vk::ImageAspectFlagBits::eColor, this->current_layout, vk::ImageLayout::eGeneral);
        wing.getGeneralCommand().buffer.end();

        vk::SubmitInfo si;
        si.setCommandBufferCount(1)
            .setPCommandBuffers(&wing.getGeneralCommand().buffer);

        _wassert_result(wing.getGraphicsQueue().submit(1, &si, wing.getGeneralCommand().fence),
                        "command submission in ResourceImage construction");

        this->current_layout = vk::ImageLayout::eGeneral;
        this->image_info = new vk::DescriptorImageInfo();
        this->image_info->setImageView(this->view)
            .setImageLayout(this->current_layout);

        _wassert_result(wing.device.waitForFences(1, &wing.getGeneralCommand().fence, true, (uint64_t)1e9),
                        "wait for operation finish in ResourceImage construction");

    }


    /*
     * TextureSetup - setup structure for Texture
     */

    TextureSetup& TextureSetup::setAddressModeX(TextureAddressMode mode) {
        this->address_mode_x = mode;
        return *this;
    }

    TextureSetup& TextureSetup::setAddressModeY(TextureAddressMode mode) {
        this->address_mode_y = mode;
        return *this;
    }

    TextureSetup& TextureSetup::setAddressModeZ(TextureAddressMode mode) {
        this->address_mode_z = mode;
        return *this;
    }

    TextureSetup& TextureSetup::setAddressMode(TextureAddressMode mode) {
        this->address_mode_x = mode;
        this->address_mode_y = mode;
        this->address_mode_z = mode;
        return *this;
    }

    TextureSetup& TextureSetup::setDepth(bool depth) {
        this->depth = depth;
        return *this;
    }

    /*
     * Texture - represents resource images with a sampler
     */

    Texture::Texture(Wingine& wing,
                     uint32_t width, uint32_t height,
                     const TextureSetup& setup) :
        Resource(vk::DescriptorType::eCombinedImageSampler) {

        vk::Format rgb_format = vk::Format::eR8G8B8A8Unorm;

        bool depth = setup.depth;

        this->wing = &wing;
        this->aspect = depth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
        vk::Device device = wing.getDevice();

        wing.cons_image_image(*this,
                              width, height,
                              depth ? vk::Format::eD32Sfloat : rgb_format,
                              vk::ImageUsageFlagBits::eSampled |
                              vk::ImageUsageFlagBits::eTransferDst,
                              vk::ImageTiling::eOptimal);
        wing.cons_image_memory(*this,
                               vk::MemoryPropertyFlagBits::eDeviceLocal);
        wing.cons_image_view(*this,
                             depth ? wImageViewDepth : wImageViewColor,
                             rgb_format);


        this->width = width;
        this->height = height;

        if (!depth) {
            Image pseudo;
            wing.cons_image_image(pseudo,
                                  width, height,
                                  depth ? vk::Format::eD32Sfloat : rgb_format,
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
            .setAddressModeU((vk::SamplerAddressMode)setup.address_mode_x)
            .setAddressModeV((vk::SamplerAddressMode)setup.address_mode_y)
            .setAddressModeW((vk::SamplerAddressMode)setup.address_mode_z)
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
    uint32_t Texture::getStride() {
        return this->stride_in_bytes;
    }

    void Texture::set(const unsigned char* pixels,
                      const std::initializer_list<SemaphoreChain*>& semaphores,
                      bool fixed_stride) {
        void* mapped_memory;
        uint32_t mem_size = this->staging_memory_memreq.size;

        vk::Device device = this->wing->getDevice();
        _wassert_result(device.mapMemory(this->staging_memory, 0, mem_size, {}, &mapped_memory),
                        "mapped memory in setting texture data");

        if (fixed_stride) {
            memcpy(mapped_memory, pixels, mem_size);
        } else {
            unsigned char* curr_mapped = (unsigned char*)mapped_memory;
            const unsigned char* curr_pixels = pixels;
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
                               this->aspect,
                               semaphores);

        this->current_staging_layout = vk::ImageLayout::eGeneral;
        this->current_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    void Texture::set(ResourceImage* image, const std::initializer_list<SemaphoreChain*>& wait_semaphores) {
        vk::ImageLayout image_final = vk::ImageLayout::eGeneral;

        this->wing->copy_image(image->width, image->height,
                               image->image, image->current_layout,
                               image_final,
                               this->width, this->height,
                               this->image, this->current_layout,
                               vk::ImageLayout::eShaderReadOnlyOptimal,
                               this->aspect,
                               wait_semaphores);

        this->current_layout = vk::ImageLayout::eShaderReadOnlyOptimal;
        image->current_layout = image_final;
    }

    void Texture::set(internal::IFramebuffer& framebuffer, const std::initializer_list<SemaphoreChain*>& wait_semaphores) {
        bool depth = this->aspect == vk::ImageAspectFlagBits::eDepth;

        internal::IImage& image = depth ? framebuffer.getDepthImage() : framebuffer.getColorImage();

        this->wing->copy_image(image.getDimensions().width,
                               image.getDimensions().height,
                               image.getImage(),
                               image.getCurrentLayout(),
                               depth ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal,
                               this->width, this->height,
                               this->image, this->current_layout,
                               vk::ImageLayout::eShaderReadOnlyOptimal,
                               this->aspect,
                               wait_semaphores);

        this->current_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

        if (depth) {
            image.setCurrentLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
        } else {
            image.setCurrentLayout(vk::ImageLayout::eColorAttachmentOptimal);
        }
    }

    StorageBuffer::StorageBuffer(Wingine& wing,
                                 int size_bytes,
                                 bool host_updatable) :
        Resource(vk::DescriptorType::eStorageBuffer) {
        this->buffer = new Buffer(wing,
                                  vk::BufferUsageFlagBits::eStorageBuffer,
                                  size_bytes,
                                  host_updatable);

        this->buffer_info = new vk::DescriptorBufferInfo();
        this->buffer_info->buffer = this->buffer->buffer;
        this->buffer_info->offset = 0;
        this->buffer_info->range = size_bytes;
    }

    void StorageBuffer::set(void* data,
                            uint32_t num_bytes,
                            uint32_t offset_bytes) {
        this->buffer->set(data,
                          num_bytes,
                          offset_bytes);
    }

};
