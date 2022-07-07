#ifndef WGUT_MODEL_HPP
#define WGUT_MODEL_HPP

#include "../declarations.hpp"
#include <vector>
#include <string>

#include "../types.hpp"

namespace wgut {

    // NB: Remember to update this whenever a new attribute is added
    const int WGUT_MODEL_ATTRIB_COUNT = 3;

    enum class VertexAttribute {
        Position,
        Normal,
        Texture
    };

    class AttribUtil {
        std::vector<int> type_indices;
        int getAttribNumber(VertexAttribute type);
    public:
        AttribUtil(const std::vector<VertexAttribute>& types);

        bool isDefined(VertexAttribute type);
        int getIndex(VertexAttribute);

    };

    class Model {
        std::vector<std::shared_ptr<wg::internal::IBuffer>> vertex_buffers;
        wg::IndexBuffer* index_buffer;

    public:
        Model(const std::vector<std::shared_ptr<wg::internal::IBuffer>>& _vertex_buffer,
              wg::IndexBuffer* _index_buffer);

        static Model fromFile(wg::Wingine& wing,
                              const std::string& file_name,
                              const std::vector<VertexAttribute>& attribs);
        static Model constructModel(wg::Wingine& wing, const std::vector<std::vector<float>>& data_buffers,
                                    const std::vector<uint32_t>& index_data);


        const std::vector<std::shared_ptr<wg::internal::IBuffer>>& getVertexBuffers();
        const wg::IndexBuffer* getIndexBuffer();


        void destroy(wg::Wingine& wing);
    };

    namespace SimpleModels {
        Model createCube(wg::Wingine& wing,
                         const std::vector<VertexAttribute>& attTypes);
        Model createSphere(wg::Wingine& wing,
                           const std::vector<VertexAttribute>& attTypes,
                           int res);
    };
};

#endif // WGUT_MODEL_HPP
