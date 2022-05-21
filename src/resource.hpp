#ifndef WG_RESOURCE_HPP
#define WG_RESOURCE_HPP

#include "declarations.hpp"
#include "image.hpp"
#include "buffer.hpp"

#include "./framebuffer/IFramebuffer.hpp"
#include "./resource/IResource.hpp"

namespace wg {
    class ResourceSet {
        vk::Device device;
        vk::DescriptorSet descriptor_set;

        ResourceSet(Wingine& wing, vk::DescriptorSetLayout layout);

    public:

        vk::DescriptorSet getDescriptorSet() const;

        void set(const std::vector<std::shared_ptr<internal::IResource>>& resources);

        friend class RenderFamily;
        friend class Wingine;
    };

    class Resource {
    protected:
        vk::DescriptorType type;
        vk::DescriptorImageInfo* image_info;
        vk::DescriptorBufferInfo* buffer_info;

        Resource(vk::DescriptorType type);

        friend class ResourceSet;

        friend class Wingine;
    };


    class ResourceImage : public Image, public Resource {
        ResourceImage(Wingine& wing,
                      uint32_t width, uint32_t height);

        friend class Wingine;
    };

    /*
     * Texture setup
     */

    enum class TextureAddressMode {
        CLAMP_TO_EDGE = (int)vk::SamplerAddressMode::eClampToEdge,
        REPEAT = (int)vk::SamplerAddressMode::eRepeat,
        MIRROR = (int)vk::SamplerAddressMode::eMirroredRepeat
    };

    struct TextureSetup {
        TextureAddressMode address_mode_x = TextureAddressMode::CLAMP_TO_EDGE;
        TextureAddressMode address_mode_y = TextureAddressMode::CLAMP_TO_EDGE;
        TextureAddressMode address_mode_z = TextureAddressMode::CLAMP_TO_EDGE;
        bool depth = false;

        TextureSetup& setAddressModeX(TextureAddressMode mode);
        TextureSetup& setAddressModeY(TextureAddressMode mode);
        TextureSetup& setAddressModeZ(TextureAddressMode mode);
        TextureSetup& setAddressMode(TextureAddressMode mode);
        TextureSetup& setDepth(bool depth);
    };
};

#endif // WG_RESOURCE_HPP
