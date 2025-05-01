#include "include/object.h"

#include <optional>
#include <string>
#include <variant>

Object::Object(int integer_data) : data(integer_data) {}

Object::Object(const std::string &string_data) : data(string_data) {}

std::optional<int> Object::asInt() const {
    if (auto ptr = std::get_if<int>(&data)) {
        return *ptr;
    }
    return std::nullopt;
}

std::optional<std::string> Object::asString() const {
    if (auto ptr = std::get_if<std::string>(&data)) {
        return *ptr;
    }
    return std::nullopt;
}
