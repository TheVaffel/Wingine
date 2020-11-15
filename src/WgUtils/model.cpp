#include "WgUtils/model.hpp"

#include "Wingine.hpp"

#include <FlatAlg.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

namespace wgut {

    AttribUtil::AttribUtil(const std::vector<ReadAttribType>& types) : type_indices(WGUT_MODEL_ATTRIB_COUNT, -1) {
        for (unsigned int i = 0; i < types.size(); i++) {
            type_indices[getAttribNumber(types[i])] = i;
        }
    }

    bool AttribUtil::isDefined(ReadAttribType type) {
        return type_indices[getAttribNumber(type)] != -1;
    }

    int AttribUtil::getIndex(ReadAttribType type) {
        return type_indices[getAttribNumber(type)];
    }

    int AttribUtil::getAttribNumber(ReadAttribType type) {
        switch(type) {
        case ReadAttribType::attTypePosition:
            return 0;
        case ReadAttribType::attTypeNormal:
            return 1;
        case ReadAttribType::attTypeTexture:
            return 2;
        default:
            std::cerr << "[AttribUtil::getAttribNumber] Unhandled attribute type" << std::endl;
            exit(-1);
        }
    }
    
    Model::Model(const std::vector<wg::Buffer*>& _vertex_buffer,
                 wg::IndexBuffer* _index_buffer) :
        vertex_buffers(_vertex_buffer),
        index_buffer(_index_buffer)
    {
        this->const_vertex_buffers =
            std::vector<const wg::Buffer*>(this->vertex_buffers.begin(),
                                           this->vertex_buffers.end());
    }

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
      
            for(unsigned int i = 0; i < data_buffers[index_pos].size() / 3; i++) {
                data_buffers[index_pos][3 * i + 0] = (data_buffers[index_pos][3 * i + 0] - min_coord) / max_diff * 2.0 - 1.0;
                data_buffers[index_pos][3 * i + 1] = (data_buffers[index_pos][3 * i + 1] - min_coord) / max_diff * 2.0 - 1.0;
                data_buffers[index_pos][3 * i + 2] = (data_buffers[index_pos][3 * i + 2] - min_coord) / max_diff * 2.0 - 1.0;
            }
        }

        

        return Model::constructModel(wing, data_buffers, index_data);
    }

    Model Model::constructModel(wg::Wingine& wing, const std::vector<std::vector<float>>& data_buffers,
                                const std::vector<uint32_t>& index_data) {
        std::vector<wg::Buffer*> buffers;
        wg::VertexBuffer<float> *buf;
        for(unsigned int i = 0; i < data_buffers.size(); i++) {
            buf = new wg::VertexBuffer<float>(wing, data_buffers[i].size());
            buf->set(data_buffers[i].data(), data_buffers[i].size());
            buffers.push_back(buf);
        }
    
        wg::IndexBuffer* index_buffer = new wg::IndexBuffer(wing, index_data.size());
        index_buffer->set(index_data.data(), index_data.size(), 0);

        return Model(buffers, index_buffer);
    }

    const std::vector<const wg::Buffer*>& Model::getVertexBuffers() {
        return this->const_vertex_buffers;
    }
  
    const wg::IndexBuffer* Model::getIndexBuffer() {
        return this->index_buffer;
    }

    void Model::destroy(wg::Wingine& wing) {
        for(unsigned int i = 0; i < this->vertex_buffers.size(); i++) {
            wing.destroy(this->vertex_buffers[i]);
        }
        wing.destroy(this->index_buffer);
    }


    namespace SimpleModels {
    
        Model createCube(wg::Wingine& wing,
                         const std::vector<ReadAttribType>& attribs) {
            AttribUtil attut(attribs);
            
            std::vector<std::vector<float>> attribute_vectors(attribs.size());
            std::vector<uint32_t> indices;

            for (int i = 0; i < 6; i++) {
                
                int dir_out = i % 3;
                int dn = (dir_out + 1) % 3;
                int dnn = (dn + 1) % 3;
                    
                int dir_comp = (i / 3) * 2 - 1;
                for (int j = 0; j < 4; j++) {    
                    
                    if (attut.isDefined(ReadAttribType::attTypePosition)) {
                        falg::Vec3 pos;
                        
                        pos[dir_out] = 0.5f * dir_comp;
                        pos[dn] = 0.5f * (j % 2 == 0 ? - 1.0f : 1.0f);
                        pos[dnn] = 0.5f * (j / 2 == 0 ? - 1.0f : 1.0f) * dir_comp;

                        attribute_vectors[attut.getIndex(ReadAttribType::attTypePosition)].push_back(pos[0]);
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypePosition)].push_back(pos[1]);
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypePosition)].push_back(pos[2]);
                    }

                    if (attut.isDefined(ReadAttribType::attTypeNormal)) {
                        falg::Vec3 norm(0.0f, 0.0f, 0.0f);
                        norm[dir_out] = dir_comp;
                        
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypeNormal)].push_back(norm[0]);
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypeNormal)].push_back(norm[1]);
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypeNormal)].push_back(norm[2]);
                    }

                    if (attut.isDefined(ReadAttribType::attTypeTexture)) {
                        falg::Vec2 tc(j % 2, j / 2);
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypeTexture)].push_back(tc[0]);
                        attribute_vectors[attut.getIndex(ReadAttribType::attTypeTexture)].push_back(tc[1]);
                    }
                }
                
                indices.push_back(4 * i + 0);
                indices.push_back(4 * i + 2);
                indices.push_back(4 * i + 1);
                indices.push_back(4 * i + 1);
                indices.push_back(4 * i + 2);
                indices.push_back(4 * i + 3);
            }

            return Model::constructModel(wing, attribute_vectors, indices);
        }
    };
};
