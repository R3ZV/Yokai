#pragma once

#include <optional>
#include <string>
#include <variant>

class Object {
    // for mow, we support ints and strings as values
    // TODO: add support for fancier types, such as JSON
    std::variant<int, std::string> data;

   public:
    Object(int integer_data);
    Object(const std::string &str_data);
    std::optional<int> asInt() const;
    std::optional<std::string> asString() const;
};
