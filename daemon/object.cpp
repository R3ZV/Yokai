#include "include/object.h"

#include <cassert>
#include <optional>
#include <print>
#include <sstream>
#include <string>
#include <variant>

Object::Object(ObjectType type, const std::string &data, int64_t timestamp)
    : timestamp(timestamp), type(type) {
    switch (this->type) {
        case STRING:
            this->data = data;
            break;
        case HASH_SET:
            auto hash_set = std::set<std::string>();
            if (!data.empty()) {
                std::string value;
                std::istringstream iss(data);
                while (iss >> value) {
                    hash_set.insert(value);
                }
            }
            std::println("HOw many times???");
            std::println("data: {}", data);
            this->data = hash_set;
            break;
    }
}

auto Object::asString() const -> std::optional<std::string> {
    if (auto ptr = std::get_if<std::string>(&data)) {
        return *ptr;
    }
    return std::nullopt;
}

auto Object::asSet() const -> std::optional<std::set<std::string>> {
    if (auto ptr = std::get_if<std::set<std::string> >(&data)) {
        return *ptr;
    }
    return std::nullopt;
}

auto Object::get_type() const -> ObjectType {
    return this->type;
}
auto Object::get_timestamp() const -> int64_t { return timestamp; }

auto Object::set_timestamp(int64_t timestamp) -> void {
    this->timestamp = timestamp;
}

auto Object::get_current_time() -> int64_t {
    return duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

auto Object::encode() const -> std::string {
    std::ostringstream oss;
    switch (this->type) {
        case STRING:
            oss << "STRING" << " " << std::get<std::string>(this->data);
            return oss.str();
        case HASH_SET:
            oss << "HASH_SET";
            auto hash_set = std::get<std::set<std::string>>(this->data);
            for (const auto& entry : hash_set) {
                oss << " " << entry;
            }
            return oss.str();
    }
}
