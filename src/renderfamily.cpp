#include "renderfamily.hpp"

#include "Wingine.hpp"

namespace wg {
  RenderFamily::RenderFamily(Wingine& wing,
			     Pipeline* pipeline,
			     bool clear,
			     int num_framebuffers) :
    wing(&wing), pipeline(pipeline) {

    if(num_framebuffers == 0) {
      num_framebuffers = wing.getNumFramebuffers();
    }
    
    this->clears = clear;
    this->num_buffers = num_framebuffers;

    this->render_passes = std::vector<vk::RenderPass>(num_framebuffers);
    for(int i = 0; i < num_framebuffers; i++) {
      if(!clear) {
	this->render_passes[i] = wing.compatibleRenderPassMap[pipeline->render_pass_type];
      } else {
	this->render_passes[i] = wing.create_render_pass(pipeline->render_pass_type,
						    clear);
      }
    }

    
    
    vk::CommandPool pool = wing.getGraphicsCommandPool();
    vk::Device device = wing.getDevice();
    
    vk::CommandBufferAllocateInfo cbi;
    cbi.setCommandPool(pool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount(1); // Premature optimization... etc.

    this->commands = std::vector<Command>(this->num_buffers);
    for(int i = 0; i < this->num_buffers; i++) {
      
      commands[i].buffer = device.allocateCommandBuffers(cbi)[0];

      commands[i].buffer
	.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

      vk::FenceCreateInfo fci;
      fci.setFlags(vk::FenceCreateFlagBits::eSignaled);
    
      commands[i].fence =
	device.createFence(fci);
    }
  }
  

  void RenderFamily::startRecording(std::vector<Framebuffer*> framebuffers) {
    
    if (framebuffers.size() == 0) {
      this->framebuffers = wing->getFramebuffers();
    } else {
      this->framebuffers = framebuffers;
    }

    _wassert((int)this->framebuffers.size() == this->num_buffers,
	     "[RenderFamily::startRecording] Number of provided framebuffers and originally declared framebuffers differ");

    for(int i = 0; i < this->num_buffers; i++) {

      vk::CommandBufferBeginInfo begin;
    
      vk::Rect2D renderRect;
      renderRect.setOffset({0, 0})
	.setExtent({this->framebuffers[i]->depthImage.width, this->framebuffers[i]->depthImage.height});
    
      vk::RenderPassBeginInfo rpb;
      rpb.setRenderPass(this->render_passes[i])
	.setClearValueCount(0)
	.setPClearValues(nullptr)
	.setFramebuffer(this->framebuffers[i]->framebuffer)
	.setRenderArea(renderRect);
    
      // Size is number of attachments
      std::vector<vk::ClearValue> clear_values;

      if(this->clears) {
	switch (this->pipeline->render_pass_type) {
	case renDepth:
	  clear_values.resize(1);
	  clear_values[0].depthStencil.depth = 1.0f;
	  clear_values[0].depthStencil.stencil = 0.0f;
	  break;
	case renColorDepth:
	  clear_values.resize(2);
	  clear_values[0].color.setFloat32({0.3f, 0.3f,
					    0.3f, 1.0f});
	  clear_values[1].depthStencil.depth = 1.0f;
	  clear_values[1].depthStencil.stencil = 0.0f;
	}

	rpb.setClearValueCount(clear_values.size())
	  .setPClearValues(clear_values.data());
      }
    
      vk::Device device = this->wing->getDevice();

      device.waitForFences(1, &this->commands[i].fence, true, (uint64_t)(1000000));
	

      this->commands[i].buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    
      this->commands[i].buffer.begin(begin);

      this->commands[i].buffer.beginRenderPass(rpb, vk::SubpassContents::eInline);
    
      this->commands[i].buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->pipeline->pipeline);
    }
  }

  void RenderFamily::recordDraw(const std::vector<Buffer*>& vertex_buffers, const IndexBuffer* ind_buf,
				const std::vector<ResourceSet*>& sets, int instanceCount){

    for(int j = 0; j < this->num_buffers; j++) {
      
      std::vector<vk::DescriptorSet> d_sets(sets.size());
      for(unsigned int i = 0; i < sets.size(); i++) {
	d_sets[i] = sets[i]->descriptor_set;
      }
	  
      if (sets.size()) {
	this->commands[j].buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
						    this->pipeline->layout,
						    0, d_sets.size(),
						    d_sets.data(),
						    0, nullptr);
      }

      std::vector<vk::Buffer> vk_buffers(vertex_buffers.size());
      std::vector<vk::DeviceSize> offsets(vertex_buffers.size());

      for(unsigned int i = 0; i < vertex_buffers.size(); i++) {
	vk_buffers[i] = vertex_buffers[i]->buffer;
	offsets[i] = 0;
      }

      this->commands[j].buffer.bindVertexBuffers(0, vk_buffers.size(),
						 vk_buffers.data(),
						 offsets.data());
      this->commands[j].buffer.bindIndexBuffer(ind_buf->buffer,
					       0, vk::IndexType::eUint32);

      this->commands[j].buffer.drawIndexed(ind_buf->num_indices, instanceCount,
					   0, 0, 0);
    }
  }

  void RenderFamily::endRecording() {
    for(int i = 0; i < this->num_buffers; i++) {
      this->commands[i].buffer.endRenderPass();
      this->commands[i].buffer.end();
    }
  }

  void RenderFamily::submit(int index) {
    if (index == -1) {
      index = this->wing->getCurrentFramebufferIndex();
    }

    _wassert(index <= this->num_buffers, "[RenderFamily::submit(int index)] Index too high. This could be because the RenderFamily instance is created with fewer buffers than there are frame buffers, and no index was explicitly set.");
    
    this->submit_command(index);
  }

  void RenderFamily::submit_command(int index) {
    vk::Device device = this->wing->getDevice();
    vk::Queue queue = this->wing->getGraphicsQueue();

    vk::PipelineStageFlags last_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo si;
    si.setPWaitDstStageMask(&last_stage)
      .setCommandBufferCount(1)
      .setPCommandBuffers(&this->commands[index].buffer)
      .setSignalSemaphoreCount(1)
      .setPSignalSemaphores(this->framebuffers[index]->has_been_drawn_semaphore);

    if (this->framebuffers[index]->ready_for_draw_semaphore != nullptr) {
      si.setWaitSemaphoreCount(1)
	.setPWaitSemaphores(this->framebuffers[index]->ready_for_draw_semaphore);
    } else {
      si.setWaitSemaphoreCount(0);
    }
    
    device.resetFences(1, &this->commands[index].fence);
    queue.submit(1, &si, this->commands[index].fence);
    
    device.waitForFences(1, &this->commands[index].fence, true, (uint64_t)1e9);
  }
};
