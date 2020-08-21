#include <Winval.hpp>
#include <Wingine.hpp>

int main() {
    const int width = 400;
    const int height = 400;

    Winval win(width, height);
    wg::Wingine wg(win);

    return 0;
}
