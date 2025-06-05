#pragma once

#include <time.h>

#include <chrono>
#include <optional>
#include <string>
#include <variant>

enum ObjectType {
    STRING,
};

class Object {
    std::variant<int, std::string> data;
    int64_t timestamp;
    ObjectType type;

   public:
    Object() = default;
    Object(ObjectType type, const std::string& data,
           int64_t timestamp = get_current_time());

    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;

    std::optional<std::string> asString() const;
    auto get_timestamp() const -> int64_t;
    auto set_timestamp(int64_t timestamp) -> void;
    auto encode() const -> std::string;

    static auto get_current_time() -> int64_t;
};
