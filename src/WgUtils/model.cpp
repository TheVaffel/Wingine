#include "WgUtils/model.hpp"

namespace wgut {
  
  Model::Model(const std::vector<wg::Buffer*>& _vertex_buffer,
	  const wg::IndexBuffer& _index_buffer) :
      vertex_buffers(_vertex_buffer),
      index_buffer(&_index_buffer)
    { }

  const std::vector<wg::Buffer*>& Model::getVertexBuffers() {
    return this->vertex_buffers;
  }
  
  const wg::IndexBuffer& Model::getIndexBuffer() {
    return *this->index_buffer;
  }
};
