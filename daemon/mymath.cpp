#include "include/mymath.h"

std::optional<int> mydiv(int a, int b) {
    if (b == 0) return std::nullopt;

    return a / b;
}
