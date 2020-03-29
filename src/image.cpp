#include "image.hpp"

#include "framebuffer.hpp"

namespace wg {
  
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
};
