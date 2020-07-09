#ifndef WG_IMAGE_HPP
#define WG_IMAGE_HPP

#include "declarations.hpp"

namespace wg {
  
  
  class Image {
  protected:
    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;
    
    vk::ImageLayout current_layout;

    uint32_t width, height;
    
    const vk::Image& getImage() const;
    const vk::DeviceMemory& getMemory() const;
    const vk::ImageView& getView() const;
    
    friend class Framebuffer;
    friend class Texture;
    friend class Wingine;
    friend class RenderFamily;
  };
  
  
}

#endif // WG_IMAGE_HPP
