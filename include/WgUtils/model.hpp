#ifndef WGUT_MODEL_HPP
#define WGUT_MODEL_HPP

#include "../Wingine.hpp"
#include <vector>

namespace wgut {
  class Model {
    std::vector<wg::Buffer*> vertex_buffers;
    const wg::IndexBuffer* index_buffer;

  public:
    Model(const std::vector<wg::Buffer*>& _vertex_buffer,
	  const wg::IndexBuffer& _index_buffer);
  
    const std::vector<wg::Buffer*>& getVertexBuffers();
    const wg::IndexBuffer& getIndexBuffer();
  };
};

#endif // WGUT_MODEL_HPP
