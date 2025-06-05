#include <cstddef>
#include <cstdint>
#include <string>

#include "../../daemon/include/command.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string input_str(reinterpret_cast<const char*>(data), size);

    auto res = Command::parse(input_str);

    return 0;
}
