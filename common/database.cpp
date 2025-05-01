#include <string>
#include "include/database.h"
#include "include/object.h"

Database::Database(){
    content={};
}

void Database::insert(std::string key, Object value){
     content[key]=value;
}

Object Database::get(std::string key){
      return content[key];
}

void Database::remove(std::string key){
     content.erase(key);
}
