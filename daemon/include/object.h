#pragma once

#include <time.h>

#include <optional>
#include <string>
#include <variant>

class Object {
    std::variant<int, std::string> data;
    // TODO: add support for fancier types, such as JSON
    // for now, we support ints and strings as values
    time_t timestamp;

   public:
    Object() = default;
    Object(int integer_data, time_t timestamp = time(nullptr));
    Object(const std::string& str_data, time_t timestamp = time(nullptr));
    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;

    std::optional<int> asInt() const;
    std::optional<std::string> asString() const;
    auto get_timestamp() const -> time_t;
    auto set_timestamp(time_t timestamp) -> void;
};
