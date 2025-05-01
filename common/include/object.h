#pragma once

#include <optional>
#include <string>
#include <variant>

#include <time.h>

class Object{
    std::variant<int, std::string> data;
    // for mow, we support ints and strings as values
    // TODO: add support for fancier types, such as JSON    
    time_t timestamp;
    public:
    Object() = default;
    Object(int integer_data, time_t timestamp = time(nullptr));
    Object(const std::string &str_data, time_t timestamp = time(nullptr));
    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;
};