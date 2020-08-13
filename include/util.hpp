#ifndef WG_UTIL_HPP
#define WG_UTIL_HPP

#include "declarations.hpp"

#include <string>

#include <vulkan/vulkan.hpp>

namespace wg {

  /*
   * Utility functions
   */

  void _wlog_error(std::string str);

  void _wassert(bool test, std::string str = "Error");
  void _wassert_result(vk::Result res, std::string str = "Error");
  uint32_t _get_memory_type_index(uint32_t type_bits,
					 vk::MemoryPropertyFlags requirements_mask,
					 vk::PhysicalDeviceMemoryProperties& mem_props);

  vk::Format _get_format(wg::ComponentType type, int num_components);
  
};

#endif // WG_UTIL_HPP
