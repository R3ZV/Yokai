#include "../lib/doctest.h"
#include "../daemon/include/mymath.h"

TEST_CASE("Valid mydiv operation") {
    REQUIRE(*mydiv(5, 2) == 2);
}
