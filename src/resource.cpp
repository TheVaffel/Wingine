#include "./resource.hpp"
#include "./Wingine.hpp"

#include "./util/log.hpp"

namespace wg {

    /* ResourceSetLayout::ResourceSetLayout(Wingine& wing, const std::vector<uint64_t>& flags) {
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

    ResourceSetLayout::ResourceSetLayout() { } */

    /* ResourceSet* Wingine::createResourceSet(const std::vector<uint64_t>& flags) {
        return new ResourceSet(*this, this->resource_set_layout_registry->ensureAndGet(flags));
        } */

    ResourceSet::ResourceSet(Wingine& wing, vk::DescriptorSetLayout layout) {
        vk::DescriptorSetAllocateInfo dsai;
        dsai.setDescriptorPool(wing.getDescriptorPool())
            .setDescriptorSetCount(1)
            .setPSetLayouts(&layout);

        vk::Device device = wing.getDevice();
        this->descriptor_set = device.allocateDescriptorSets(dsai)[0];
        this->device = device;
    }

    void ResourceSet::set(const std::vector<std::shared_ptr<internal::IResource>>& resources) {
        std::vector<vk::WriteDescriptorSet> writes(resources.size());

        std::vector<std::unique_ptr<internal::IResourceWriteAuxillaryData>> aux_data(resources.size());
        for(unsigned int i = 0; i < resources.size(); i++) {
            writes[i].setDstSet(this->descriptor_set)
                .setDstBinding(i);
            aux_data[i] = resources[i]->writeDescriptorUpdate(writes[i]);
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

};
