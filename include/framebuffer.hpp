#ifndef WG_FRAMEBUFFER_HPP
#define WG_FRAMEBUFFER_HPP

#include "declarations.hpp"

#include "image.hpp"

namespace wg {

  class Framebuffer {
    Image colorImage;
    Image depthImage;
    vk::Framebuffer framebuffer;
    
    Framebuffer(Wingine& wing,
		 int width, int height,
		 bool depthOnly);
    Framebuffer();
    
  public:
    
    void destroy();

    const Image& getColorImage() const;
    const Image& getDepthImage() const;
    const vk::Framebuffer& getFramebuffer() const;

    friend class RenderFamily;
    friend class Wingine;
    friend class Texture;
    friend class RenderFamily;
  };
};

#endif // WG_FRAMEBUFFER_HPP
