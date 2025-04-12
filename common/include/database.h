#pragma once

#include "object.h"
#include<map>
#include<string>
#include <expected>
#include <system_error>

class Database{
    std::map<std::string, Object> data;
public:
    void insert(std::string key, const Object& value);
    std::expected<Object, std::error_code> select(std::string key);
    void show_objects();
};