#pragma once

#include "../resource/IResource.hpp"
#include "./IImage.hpp"

namespace wg::internal {
    class ITexture : public virtual IResource, public virtual IImage {

    public:
        virtual vk::Sampler getSampler() const = 0;

        virtual ~ITexture() = default;
    };
};
