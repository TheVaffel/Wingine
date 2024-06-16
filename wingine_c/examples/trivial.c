#include "../wingine_c.h"

int main() {
  win_winval_t *win = win_create_winval(400, 400);
  wg_wingine_t *wing = wg_create_wingine_with_winval(win, "trivial example");

  wg_destroy_wingine(wing);
}
