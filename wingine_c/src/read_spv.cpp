#include "./read_spv.h"

#include <fstream>
#include <iostream>

extern "C" {

    uint32_t* wg_read_spv(const char* file_name, uint32_t* out_buffer_length) {
        std::string str(file_name);
        std::ifstream ifs(str, std::ifstream::in);
        if (!ifs) {
            std::cout << "[wg_spv] Did not find vertex spirv " << str << std::endl;
            exit(-1);
        }

        ifs.seekg (0, ifs.end);
        int length = ifs.tellg();
        ifs.seekg (0, ifs.beg);

        uint32_t num_elements = length / sizeof(uint32_t);
        uint32_t* spv = new uint32_t[num_elements];

        ifs.read((char*)spv, length);
        ifs.close();

        *out_buffer_length = num_elements;
        return spv;
    }

    void wg_free_spv(uint32_t* spv) {
        delete[] spv;
    }
};
