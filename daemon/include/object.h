#pragma once

#include <time.h>

#include <chrono>
#include <optional>
#include <string>
#include <variant>
#include <set>

enum ObjectType {
    STRING,  HASH_TABLE, 
};

class Object {
    std::variant<int, std::string, std::set<std::string> > data;
    int64_t timestamp;
    ObjectType type;

   public:
    Object() = default;
    Object(ObjectType type, const std::string& data,
           int64_t timestamp = get_current_time());
    Object(ObjectType type, int64_t timestamp=get_current_time()); //this will just make the set
    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;

    std::optional<std::string> asString() const;
    auto asSet() const -> std::optional<std::set<std::string> >;
    auto get_timestamp() const -> int64_t;
    auto set_timestamp(int64_t timestamp) -> void;
    auto encode() const -> std::string;

    static auto get_current_time() -> int64_t;
};
