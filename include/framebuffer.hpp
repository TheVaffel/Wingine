#ifndef WG_FRAMEBUFFER_HPP
#define WG_FRAMEBUFFER_HPP

#include "declarations.hpp"

#include "image.hpp"

namespace wg {

  class _Framebuffer {
    Image colorImage;
    Image depthImage;
    vk::Framebuffer framebuffer;
    vk::Semaphore *ready_for_draw_semaphore,
      *has_been_drawn_semaphore;
    
    _Framebuffer(Wingine& wing,
		 int width, int height,
		 bool depthOnly, bool withoutSemaphore = false);
    _Framebuffer();
    
  public:
    
    void destroy();
    
    const Image& getColorImage() const;
    const Image& getDepthImage() const;
    const vk::Framebuffer& getFramebuffer() const;

    friend class RenderFamily;
    friend class Wingine;
    friend class _Texture;
    friend class RenderFamily;
  };
};

#endif // WG_FRAMEBUFFER_HPP
