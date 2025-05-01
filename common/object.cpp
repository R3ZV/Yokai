#include "include/object.h"

#include <optional>
#include <string>
#include <variant>

Object::Object(int integer_data, time_t timestamp)
    : data(integer_data), timestamp(timestamp) {}

Object::Object(const std::string &string_data, time_t timestamp)
    : data(string_data), timestamp(timestamp) {}

auto Object::asInt() const -> std::optional<int> {
    if (auto ptr = std::get_if<int>(&data)) {
        return *ptr;
    }
    return std::nullopt;
}

auto Object::asString() const -> std::optional<std::string> {
    if (auto ptr = std::get_if<std::string>(&data)) {
        return *ptr;
    }
    return std::nullopt;
}

auto Object::get_timestamp() const -> time_t { return timestamp; }

auto Object::set_timestamp(time_t timestamp) -> void {
    this->timestamp = timestamp;
}
