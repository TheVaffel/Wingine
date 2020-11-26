#ifndef WGUT_MODEL_HPP
#define WGUT_MODEL_HPP

#include "../declarations.hpp"
#include <vector>
#include <string>

namespace wgut {

    // NB: Remember to update this whenever a new attribute is added
    const int WGUT_MODEL_ATTRIB_COUNT = 3;
    
    enum class ReadAttribType {
        attTypePosition,
        attTypeNormal,
        attTypeTexture
    };

    class AttribUtil {
        std::vector<int> type_indices;
        int getAttribNumber(ReadAttribType type);
    public:
        AttribUtil(const std::vector<ReadAttribType>& types);

        bool isDefined(ReadAttribType type);
        int getIndex(ReadAttribType);

    };
  
    class Model {
        std::vector<wg::Buffer*> vertex_buffers;
        std::vector<const wg::Buffer*> const_vertex_buffers;
        wg::IndexBuffer* index_buffer;

    public:
        Model(const std::vector<wg::Buffer*>& _vertex_buffer,
              wg::IndexBuffer* _index_buffer);

        static Model fromFile(wg::Wingine& wing,
                              const std::string& file_name,
                              const std::vector<ReadAttribType>& attribs);
        static Model constructModel(wg::Wingine& wing, const std::vector<std::vector<float>>& data_buffers,
                                    const std::vector<uint32_t>& index_data);

        
        const std::vector<const wg::Buffer*>& getVertexBuffers();
        const wg::IndexBuffer* getIndexBuffer();


        void destroy(wg::Wingine& wing);
    };

    namespace SimpleModels {
        Model createCube(wg::Wingine& wing,
                         const std::vector<ReadAttribType>& attTypes);
        Model createSphere(wg::Wingine& wing,
                           const std::vector<ReadAttribType>& attTypes,
                           int res);
    };
};

#endif // WGUT_MODEL_HPP
