#include "image.hpp"

#include "framebuffer.hpp"

#include "Wingine.hpp"

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

  const Image& Framebuffer::getColorImage() const {
    return this->colorImage;
  }

  const Image& Framebuffer::getDepthImage() const {
    return this->depthImage;
  }

  void Image::constructImage(Wingine& wing,
			     Image& image,
			     int width, int height,
			     vk::Format format,
			     vk::ImageUsageFlags usage,
			     vk::ImageTiling tiling,
			     vk::MemoryPropertyFlags mem_flags) {
    wing.cons_image_image(image,
			  width, height,
			  format,
			  usage,
			  tiling);
    wing.cons_image_memory(image,
		      mem_flags);
    wing.cons_image_view(image,
			  wImageViewColor);
  }
  
};
