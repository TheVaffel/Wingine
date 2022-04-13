#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

namespace wg {
    void _wlog_error(const std::string& str);
    void _wlog_warn(const std::string& str);

    void _wassert(bool test, const std::string& str = "Error");
    void _wassert_result(vk::Result res, const std::string& str = "Error");
};
