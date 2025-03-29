#pragma once
#include "object.h"
#include<map>
#include<string>
class Database{
    std::map<std::string, Object> data;
public:
    void insert(std::string key, Object value);
    Object select(std::string key);
};