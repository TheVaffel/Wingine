#include "renderfamily.hpp"

#include "Wingine.hpp"

namespace wg {
  RenderFamily::RenderFamily(Wingine& wing,
			     Pipeline& pipeline,
			     bool clear) :
    wing(&wing), pipeline(&pipeline) {

    this->clears = clear;

    if(!clear) {
      this->render_pass = wing.compatibleRenderPassMap[pipeline.render_pass_type];
    } else {
      this->render_pass = wing.create_render_pass(pipeline.render_pass_type,
						  clear);
    }

    
    
    vk::CommandPool pool = wing.getGraphicsCommandPool();
    vk::Device device = wing.getDevice();
    
    vk::CommandBufferAllocateInfo cbi;
    cbi.setCommandPool(pool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount(1); // Premature optimization... etc.

    command.buffer = device.allocateCommandBuffers(cbi)[0];

    command.buffer
      .reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    vk::FenceCreateInfo fci;
    fci.setFlags(vk::FenceCreateFlagBits::eSignaled);
    
    command.fence =
      device.createFence(fci);
  }
  
  RenderObject::RenderObject(const std::vector<_VertexBuffer*>& buffers,
			     IndexBuffer nindexBuffer) :
    vertexBuffers(buffers), indexBuffer(nindexBuffer) { }

  

  void RenderFamily::startRecording(_Framebuffer* framebuffer) {

    if (framebuffer == nullptr) {
      framebuffer = wing->getCurrentFramebuffer();
    }
    
    vk::CommandBufferBeginInfo begin;
    
    vk::Rect2D renderRect;
    renderRect.setOffset({0, 0})
      .setExtent({framebuffer->depthImage.width, framebuffer->depthImage.height});
    
    vk::RenderPassBeginInfo rpb;
    rpb.setRenderPass(this->render_pass)
      .setClearValueCount(0)
      .setPClearValues(nullptr)
      .setFramebuffer(framebuffer->framebuffer)
      .setRenderArea(renderRect);

    this->current_framebuffer = framebuffer;
    
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
    
    device.waitForFences(1, &this->command.fence, true, (uint64_t)(1000000));

    this->command.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
    
    this->command.buffer.begin(begin);

    this->command.buffer.beginRenderPass(rpb, vk::SubpassContents::eInline);
    
    this->command.buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->pipeline->pipeline);
  }

  void RenderFamily::recordDraw(RenderObject& obj,
				std::vector<ResourceSet> sets) {
    std::vector<vk::DescriptorSet> d_sets(sets.size());
    for(unsigned int i = 0; i < sets.size(); i++) {
      d_sets[i] = sets[i].descriptor_set;
    }

    if (sets.size()) {
      this->command.buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
					      this->pipeline->layout,
					      0, d_sets.size(),
					      d_sets.data(),
					      0, nullptr);
    }

    std::vector<vk::Buffer> vertexBuffers(obj.vertexBuffers.size());
    std::vector<vk::DeviceSize> offsets(obj.vertexBuffers.size());

    for(unsigned int i = 0; i < vertexBuffers.size(); i++) {
      vertexBuffers[i] = obj.vertexBuffers[i]->buffer;
      offsets[i] = 0;
    }

    this->command.buffer.bindVertexBuffers(0, vertexBuffers.size(),
					   vertexBuffers.data(),
					   offsets.data());
    this->command.buffer.bindIndexBuffer(obj.indexBuffer.buffer,
					 0, vk::IndexType::eUint32);

    this->command.buffer.drawIndexed(obj.indexBuffer.num_indices, 1,
				     0, 0, 0);
  }

  void RenderFamily::endRecording() {
    this->command.buffer.endRenderPass();
    this->command.buffer.end();

    this->submit_command();
  }

  void RenderFamily::submit_command() {
    vk::Device device = this->wing->getDevice();
    vk::Queue queue = this->wing->getGraphicsQueue();

    vk::PipelineStageFlags last_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo si;
    si.setPWaitDstStageMask(&last_stage)
      .setCommandBufferCount(1)
      .setPCommandBuffers(&this->command.buffer)
      .setSignalSemaphoreCount(1)
      .setPSignalSemaphores(this->current_framebuffer->has_been_drawn_semaphore);

    if (this->current_framebuffer->ready_for_draw_semaphore != nullptr) {
      si.setWaitSemaphoreCount(1)
	.setPWaitSemaphores(this->current_framebuffer->ready_for_draw_semaphore);
    } else {
      si.setWaitSemaphoreCount(0);
    }
    
    device.resetFences(1, &this->command.fence);
    queue.submit(1, &si, this->command.fence);
    
    device.waitForFences(1, &this->command.fence, true, (uint64_t)1e9);
  }
};
