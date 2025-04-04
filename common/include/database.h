#pragma once
#include "object.h"
#include<map>
#include<string>
class Database{
    std::map<std::string, Object> content;
public:
    void insert(std::string key, Object value);
    Object get(std::string key);
    void remove(std::string key); 
};