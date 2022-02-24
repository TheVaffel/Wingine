#include "./log.hpp"

#include <iostream>

namespace wg {

    void _wlog_error(const std::string& str) {
        std::cout << "[Wingine Error] " << str << std::endl;
        throw std::runtime_error(str);
    }

    void _wlog_warn(const std::string& str) {
        std::cerr << "[Wingine Warning] " << str << std::endl;
    }

    void _wassert(bool test, const std::string& str) {
        if(!test) {
            _wlog_error(str);
            throw std::runtime_error(str);
        }
    }

    void _wassert_result(vk::Result res, const std::string& str) {
        if( res != vk::Result::eSuccess ) {
            _wlog_error(str);
            throw std::runtime_error(str);
        }
    }
};
