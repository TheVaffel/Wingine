#include "../wingine_c.h"

int main() {
  wg_wingine_t wing;
  wg_initialize_wingine(&wing, 400, 400);

  wg_destroy_wingine(&wing);
}
