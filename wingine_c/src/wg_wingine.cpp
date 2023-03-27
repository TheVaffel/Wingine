#include "./wg_wingine.h"
#include <Wingine.hpp>

extern "C" {

  int wg_initialize_wingine(wg_wingine_t* wing, uint32_t width, uint32_t height) {
    wing->winval = (void*) new Winval(width, height);
    wing->wingine = (void*) new wg::Wingine(*reinterpret_cast<Winval*>(wing->winval));

    return 0;
  }

  int wg_destroy_wingine(wg_wingine_t* wing) {
    delete reinterpret_cast<wg::Wingine*>(wing->wingine);
    delete reinterpret_cast<Winval*>(wing->winval);

    return 0;
  }
};
