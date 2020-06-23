#include "WgUtils/model.hpp"

#include "Wingine.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace wgut {
  
  Model::Model(const std::vector<wg::Buffer*>& _vertex_buffer,
	       wg::IndexBuffer& _index_buffer) :
    vertex_buffers(_vertex_buffer),
    index_buffer(&_index_buffer)
  { }

  Model Model::fromFile(wg::Wingine& wing,
		  const std::string& file_name,
		  const std::vector<ReadAttribType>& atts) {
    std::ifstream file;
    file.open(file_name);

    if(!file.is_open()) {
      std::cerr << "Could not open file " << file_name << std::endl;
      exit(-1);
    }

    std::string line;
    std::string type;

    int index_pos = -1,
      index_norm = -1,
      index_tex = -1;

    std::vector<std::vector<float> > data_buffers(atts.size());
    std::vector<uint32_t> index_data;
   
    for(unsigned int i = 0; i < atts.size(); i++) {
      if(atts[i] == ReadAttribType::attTypePosition) {
	index_pos = i;
      } else if (atts[i] == ReadAttribType::attTypeNormal) {
	index_norm = i;
      } else if (atts[i] == ReadAttribType::attTypeTexture) {
	index_tex = i;
      } else {
	std::cerr << "[Model::fromFile] Unrecognized attrib type" << std::endl;
	exit(-1);
      }
    }

    float a, b, c;
    float maxa = 0.0f, maxb = 0.0f, maxc = 0.0f;
    float mina = 0.0f, minb = 0.0f, minc = 0.0f;
    int aa, bb, cc;
    
    while(std::getline(file, line)) {
      std::istringstream iss(line);
      iss >> type;

      if(type[0] == '#') {
	continue;
      }

      if(type == "v") {
	if(index_pos != -1) {
	  iss >> a >> b >> c;
	  maxa = std::max(maxa, a);
	  maxb = std::max(maxb, b);
	  maxc = std::max(maxc, c);
	  mina = std::min(mina, a);
	  minb = std::min(minb, b);
	  minc = std::min(minc, c);
	  data_buffers[index_pos].push_back(a);
	  data_buffers[index_pos].push_back(b);
	  data_buffers[index_pos].push_back(c);
	}
      } else if( type == "vn") {
	if(index_norm != -1) {
	  iss >> a >> b >> c;
	  data_buffers[index_norm].push_back(a);
	  data_buffers[index_norm].push_back(b);
	  data_buffers[index_norm].push_back(c);
	}
	
      } else if(type == "vt") {
	if(index_tex != -1) {
	  iss >> a >> b;
	  data_buffers[index_tex].push_back(a);
	  data_buffers[index_tex].push_back(b);
	}
      } else if(type == "f") {
	
	iss >> aa >> bb >> cc;
	
	index_data.push_back(aa - 1);
	index_data.push_back(bb - 1);
	index_data.push_back(cc - 1);
      } else {
	std::cerr << "Could parse line \"" << line << "\", skipping" << std::endl;
      }
    }

    // Here, we normalize positions, for debugging
    if(index_pos != -1) {
      float max_diff = maxa - mina;
      float min_coord = mina;
      if(maxb - minb > max_diff) {
	max_diff = maxb - minb;
	min_coord = minb;
      }
      if(maxc - minc > max_diff) {
	max_diff = maxc - minc;
	min_coord = minc;
      }
      
      for(int i = 0; i < data_buffers[index_pos].size() / 3; i++) {
	data_buffers[index_pos][3 * i + 0] = (data_buffers[index_pos][3 * i + 0] - min_coord) / max_diff * 2.0 - 1.0;
	data_buffers[index_pos][3 * i + 1] = (data_buffers[index_pos][3 * i + 1] - min_coord) / max_diff * 2.0 - 1.0;
	data_buffers[index_pos][3 * i + 2] = (data_buffers[index_pos][3 * i + 2] - min_coord) / max_diff * 2.0 - 1.0;
	}
    }
    
    std::vector<wg::Buffer*> buffers;
    wg::VertexBuffer<float> *buf;
    for(unsigned int i = 0; i < data_buffers.size(); i++) {
      buf = new wg::VertexBuffer<float>(wing, data_buffers[i].size());
      buf->set(data_buffers[i].data(), data_buffers[i].size());
      buffers.push_back(buf);
    }
    
    wg::IndexBuffer* index_buffer = new wg::IndexBuffer(wing, index_data.size());
    index_buffer->set(index_data.data(), index_data.size(), 0);

    return Model(buffers, *index_buffer);
  }

  const std::vector<wg::Buffer*>& Model::getVertexBuffers() {
    return this->vertex_buffers;
  }
  
  const wg::IndexBuffer& Model::getIndexBuffer() {
    return *this->index_buffer;
  }

  void Model::destroy(wg::Wingine& wing) {
    for(unsigned int i = 0; i < this->vertex_buffers.size(); i++) {
      wing.destroy(*(wg::VertexBuffer<float>*)this->vertex_buffers[i]);
    }
    wing.destroy(*this->index_buffer);
  }
};
