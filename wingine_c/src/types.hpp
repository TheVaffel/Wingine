#pragma once

#include <types.h>
#include <memory>

struct wg_generic_shared_ptr_t {
  std::shared_ptr<void> sp;
};
