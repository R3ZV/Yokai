#pragma once

#include <time.h>

#include <chrono>
#include <optional>
#include <set>
#include <string>
#include <variant>

enum ObjectType {
    STRING,
    HASH_SET,
};

class Object {
    std::variant<int, std::string, std::set<std::string>> data;
    int64_t timestamp;
    ObjectType type;

   public:
    Object() = default;
    Object(ObjectType type, const std::string& data,
           int64_t timestamp = get_current_time());
    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;

    std::optional<std::string> asString() const;
    auto asSet() const -> std::optional<std::set<std::string>>;
    auto insert(std::string value) -> void;
    auto get_timestamp() const -> int64_t;
    auto get_type() const -> ObjectType;
    auto set_timestamp(int64_t timestamp) -> void;
    auto encode() const -> std::string;

    static auto get_current_time() -> int64_t;
};
