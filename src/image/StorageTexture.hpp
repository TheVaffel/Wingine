#pragma once

#include "./BasicImage.hpp"
#include "./BasicTexture.hpp"

namespace wg::internal {

    class StorageTexture : public virtual IImage, protected BasicTexture {

        /**
         * Represents the texture resource for a storage texture
         */
        class TextureResourceComponent : public IResource {
            std::shared_ptr<StorageTexture> parent_image;

        public:
            TextureResourceComponent(std::shared_ptr<StorageTexture> parent_ref);

            [[nodiscard]]
            virtual std::unique_ptr<IResourceWriteAuxillaryData>
            writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const final;
        };

        /**
         * Represents the storage image resource for a storage texture
         */
        class StorageImageResourceComponent : public IResource {
            std::shared_ptr<StorageTexture> parent_image;

        public:
            StorageImageResourceComponent(std::shared_ptr<StorageTexture> parent_ref);

            [[nodiscard]]
            virtual std::unique_ptr<IResourceWriteAuxillaryData>
            writeDescriptorUpdate(vk::WriteDescriptorSet& write_info) const final;
        };


        std::weak_ptr<TextureResourceComponent> texture_resource;
        std::weak_ptr<StorageImageResourceComponent> storage_image_resource;

        std::weak_ptr<StorageTexture> self_reference;

        std::shared_ptr<CommandManager> command_manager;

        Command command;
        vk::Queue queue;

        StorageTexture(const vk::Extent2D& dimensions,
                       std::shared_ptr<DeviceManager> device_manager,
                       std::shared_ptr<CommandManager> command_manager,
                       std::shared_ptr<QueueManager> queue_manager);

    public:

        /**
         * Since Storage texture needs a weak self-reference, it is encapsulated
         * behind a create-method
         */
        static std::shared_ptr<StorageTexture> createStorageTexture(const vk::Extent2D& dimensions,
                                                                    std::shared_ptr<DeviceManager> device_manager,
                                                                    std::shared_ptr<CommandManager> command_manager,
                                                                    std::shared_ptr<QueueManager> queue_manager);

        std::shared_ptr<TextureResourceComponent> getTexture();
        std::shared_ptr<StorageImageResourceComponent> getStorageImage();

        void makeIntoTextureSync();
        void makeIntoStorageImageSync();

        ~StorageTexture();
    };
};
