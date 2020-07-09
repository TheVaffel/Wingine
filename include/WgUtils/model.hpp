#ifndef WGUT_MODEL_HPP
#define WGUT_MODEL_HPP

#include "../declarations.hpp"
#include <vector>
#include <string>

namespace wgut {

  enum class ReadAttribType {
    attTypePosition,
    attTypeNormal,
    attTypeTexture
  };
  
  class Model {
    std::vector<wg::Buffer*> vertex_buffers;
    wg::IndexBuffer* index_buffer;

  public:
    Model(const std::vector<wg::Buffer*>& _vertex_buffer,
	  wg::IndexBuffer* _index_buffer);

    static Model fromFile(wg::Wingine& wing,
			  const std::string& file_name,
			  const std::vector<ReadAttribType>& attribs);
    
    const std::vector<wg::Buffer*>& getVertexBuffers();
    const wg::IndexBuffer* getIndexBuffer();

    void destroy(wg::Wingine& wing);
  };
};

#endif // WGUT_MODEL_HPP
