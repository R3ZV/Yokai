#include "include/object.h"

#include <optional>
#include <string>
#include <variant>

Object::Object(int integer_data, int64_t timestamp)
    : data(integer_data), timestamp(timestamp) {}

Object::Object(const std::string &string_data, int64_t timestamp)
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

auto Object::get_timestamp() const -> int64_t { return timestamp; }

auto Object::set_timestamp(int64_t timestamp) -> void {
    this->timestamp = timestamp;
}

auto Object::get_current_time() -> int64_t {
    return duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}