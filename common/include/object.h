#pragma once

#include<variant>
#include<string>
#include<optional>
class Object{
    std::variant<int, std::string> data; //for mow, we support ints and strings as values
    //TODO: add support for fancier types, such as JSON
    public:
    Object(int integer_data);
    Object(const std::string &str_data);
    std::optional<int> asInt() const;
    std::optional<std::string> asString() const;
};