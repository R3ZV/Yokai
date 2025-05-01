#pragma once

#include <map>
#include <string>

#include "object.h"

class Database {
    std::map<std::string, Object> content;
    Database();

   public:
    void insert(std::string key, Object value);
    Object get(std::string key);
    void remove(std::string key);
};
