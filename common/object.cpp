#include "include/object.h"

#include <optional>
#include <string>
#include <variant>

Object::Object(int integer_data, time_t timestamp)
    : data(integer_data), timestamp(timestamp) {}

Object::Object(const std::string &string_data, time_t timestamp)
    : data(string_data), timestamp(timestamp) {}

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

time_t Object::get_timestamp() const { return timestamp; }

void Object::set_timestamp(time_t timestamp) { this->timestamp = timestamp; }