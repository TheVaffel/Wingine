#include "../wingine_c.h"

int main() {
  wg_wingine_t *wing = wg_create_wingine(400, 400);

  wg_destroy_wingine(wing);
}
