#include "./StorageTexture.hpp"

#include "../resource/ResourceWriteImageAuxillaryData.hpp"

#include "./imageUtil.hpp"

namespace wg::internal {

    /**
     * TextureResourceComponent
     */
    StorageTexture::TextureResourceComponent::TextureResourceComponent(std::shared_ptr<StorageTexture> parent_ref)
        : parent_image(parent_ref) { }

    std::unique_ptr<IResourceWriteAuxillaryData>
    StorageTexture::TextureResourceComponent::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteImageAuxillaryData>();
        aux_data->image_write_info.sampler = parent_image->getSampler();
        aux_data->image_write_info.imageView = parent_image->getView();
        aux_data->image_write_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo(&aux_data->image_write_info)
            .setDstArrayElement(0);

        return aux_data;
    }

    /**
     * StorageImageResourceComponent
     */
    StorageTexture::StorageImageResourceComponent::StorageImageResourceComponent(std::shared_ptr<StorageTexture> parent_ref)
        : parent_image(parent_ref) { }


    std::unique_ptr<IResourceWriteAuxillaryData>
    StorageTexture::StorageImageResourceComponent::writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const {
        auto aux_data = std::make_unique<ResourceWriteImageAuxillaryData>();
        aux_data->image_write_info.imageView = parent_image->getView();
        aux_data->image_write_info.imageLayout = vk::ImageLayout::eGeneral;

        write_info
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eStorageImage)
            .setPImageInfo(&aux_data->image_write_info)
            .setDstArrayElement(0);

        return aux_data;
    }

    /**
     * Storage Texture
     */
    StorageTexture::StorageTexture(const vk::Extent2D& dimensions,
                                   std::shared_ptr<DeviceManager> device_manager,
                                   std::shared_ptr<CommandManager> command_manager,
                                   std::shared_ptr<QueueManager> queue_manager)
        : BasicTexture(dimensions,
                       BasicTextureSetup(BasicImageSetup::createStorageTextureSetup()),
                       device_manager),
          command_manager(command_manager)
    {
        this->command = command_manager->createGraphicsCommands(1)[0];
        this->queue = queue_manager->getGraphicsQueue();

        imageUtil::initializeLayout(*this,
                                    vk::ImageLayout::eGeneral,
                                    command,
                                    queue,
                                    this->device_manager->getDevice());
    }


    std::shared_ptr<StorageTexture>
    StorageTexture::createStorageTexture(const vk::Extent2D& dimensions,
                                         std::shared_ptr<DeviceManager> device_manager,
                                         std::shared_ptr<CommandManager> command_manager,
                                         std::shared_ptr<QueueManager> queue_manager) {
        std::shared_ptr<StorageTexture> st =
            std::shared_ptr<StorageTexture>(new StorageTexture(dimensions,
                                                               device_manager,
                                                               command_manager,
                                                               queue_manager));
        st->self_reference = std::weak_ptr(st);
        return st;
    }


    void StorageTexture::makeIntoTextureSync() {
        imageUtil::setLayout(*this,
                             vk::ImageLayout::eGeneral,
                             vk::ImageLayout::eShaderReadOnlyOptimal,
                             this->command,
                             this->queue,
                             this->device_manager->getDevice());
    }

    void StorageTexture::makeIntoStorageImageSync() {
        imageUtil::setLayout(*this,
                             vk::ImageLayout::eShaderReadOnlyOptimal,
                             vk::ImageLayout::eGeneral,
                             this->command,
                             this->queue,
                             this->device_manager->getDevice());
    }

    std::shared_ptr<StorageTexture::StorageImageResourceComponent> StorageTexture::getStorageImage() {
        if (this->storage_image_resource.expired()) {
            std::shared_ptr<StorageTexture::StorageImageResourceComponent> sic =
                std::make_shared<StorageTexture::StorageImageResourceComponent>(this->self_reference.lock());
            this->storage_image_resource = std::weak_ptr(sic);
            return sic;
        }

        return this->storage_image_resource.lock();
    }

    std::shared_ptr<StorageTexture::TextureResourceComponent> StorageTexture::getTexture() {
        if (this->texture_resource.expired()) {
            std::shared_ptr<StorageTexture::TextureResourceComponent> tc =
                std::make_shared<StorageTexture::TextureResourceComponent>(this->self_reference.lock());
            this->texture_resource = std::weak_ptr(tc);
            return tc;
        }

        return this->texture_resource.lock();
    }

    StorageTexture::~StorageTexture() {
        command_manager->destroyGraphicsCommands({this->command});
    }
};
