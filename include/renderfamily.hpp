#ifndef WG_RENDERFAMILY_HPP
#define WG_RENDERFAMILY_HPP

#include "declarations.hpp"

#include "pipeline.hpp"
#include "resource.hpp"

namespace wg {

  class RenderFamily {
    Wingine* wing;

    std::vector<Command> commands;
    Pipeline* pipeline;
    std::vector<vk::RenderPass> render_passes;
    bool clears;
    int num_buffers;
    std::vector<Framebuffer*> framebuffers;
    
    RenderFamily(Wingine& wing,
		 Pipeline* pipeline,
		 bool clear,
		 int num_buffers = 0);

    void submit_command(const std::initializer_list<SemaphoreChain*>& wait_semaphores, int index);
    
  public:
    void startRecording(std::vector<Framebuffer*> framebuffer = {});

    void recordDraw(const std::vector<Buffer*>& buffers, const IndexBuffer* ind_buf,
		    const std::vector<ResourceSet*>& sets, int instanceCount = 1);
    void endRecording();

    void submit(const std::initializer_list<SemaphoreChain*>& wait_semaphores = {}, int index = -1);
    
    friend class Wingine;
  };

  class ComputeFamily {
    Wingine* wing;

    ComputeFamily(Wingine& wing,
		  Pipeline* pipeline);
    
  public:

    void startRecording();
    void recordCompute(const std::vector<ResourceSet*>& set, int x_extent, int y_extent, int z_extent);
    void endRecording();

    void submit(const std::initializer_list<SemaphoreChain*>& wait_semaphores = {}, int index = -1);

    friend class Wingine;
  };
};
#endif // WG_RENDERFAMILY_HPP
