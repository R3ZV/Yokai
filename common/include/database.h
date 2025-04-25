#pragma once

#include "object.h"
#include<map>
#include<string>
#include <expected>
#include <system_error>

class Database{
    std::map<std::string, Object> data;
public:
    std::optional<std::error_code> insert_key(const std::string& key, const Object& value);
    std::optional<std::error_code> delete_key(const std::string& key);
    std::expected<Object, std::error_code> select(const std::string& key);
    void show_objects();
};