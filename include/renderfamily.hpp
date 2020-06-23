#ifndef WG_RENDERFAMILY_HPP
#define WG_RENDERFAMILY_HPP

#include "declarations.hpp"

#include "pipeline.hpp"
#include "resource.hpp"

namespace wg {
  
  struct Command {
    vk::CommandBuffer buffer;
    vk::Fence fence;
  };

  class RenderFamily {
    Wingine* wing;

    Command command;
    Pipeline* pipeline;
    vk::RenderPass render_pass;
    _Framebuffer *current_framebuffer;
    bool clears;
    
    RenderFamily(Wingine& wing,
		 Pipeline& pipeline,
		 bool clear);

    void submit_command();
    
  public:
    void startRecording(_Framebuffer* framebuffer = nullptr);

    void recordDraw(const std::vector<Buffer*>& buffers, const IndexBuffer& ind_buf,
		    const std::vector<ResourceSet>& sets, int instanceCount = 1);
    void endRecording();
    
    friend class Wingine;
  };
};
#endif // WG_RENDERFAMILY_HPP
