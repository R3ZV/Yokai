#pragma once

#include <time.h>
#include <chrono>
#include <optional>
#include <string>
#include <variant>

class Object {
    std::variant<int, std::string> data;
    // TODO: add support for fancier types, such as JSON
    // for now, we support ints and strings as values
    int64_t timestamp;

   public:
    Object() = default;
    Object(int integer_data, int64_t timestamp = get_current_time());
    Object(const std::string& str_data, int64_t timestamp = get_current_time());
    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;

    std::optional<int> asInt() const;
    std::optional<std::string> asString() const;
    auto get_timestamp() const -> int64_t;
    auto set_timestamp(int64_t timestamp) -> void;

    static auto get_current_time() -> int64_t;
};
