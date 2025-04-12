#pragma once

#include<variant>
#include<string>
#include<optional>

class Object{
    std::variant<int, std::string> data; //for mow, we support ints and strings as values
    //TODO: add support for fancier types, such as JSON
    public:
    Object() = default;
    Object(int integer_data);
    Object(const std::string &str_data);
    Object(const Object& obj) = default;
    Object& operator=(const Object& obj) = default;
    std::optional<int> asInt() const;
    std::optional<std::string> asString() const;
};