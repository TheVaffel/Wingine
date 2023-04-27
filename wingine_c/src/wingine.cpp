#include "./wingine.h"
#include <Wingine.hpp>

#include "./types.hpp"

extern "C" {

  wg_wingine_t* wg_create_wingine(uint32_t width, uint32_t height) {
      std::unique_ptr<Winval> win = std::make_unique<Winval>(width, height);
      return new wg_wingine_t {
          .wingine = wg::Wingine(*win),
          .winval = std::move(win)
      };
  }

  void wg_destroy_wingine(wg_wingine_t* wing) {
      delete wing;
  }
};
