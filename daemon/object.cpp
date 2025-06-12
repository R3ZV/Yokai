#include "include/object.h"

#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <sstream>
Object::Object(ObjectType type, const std::string &data, int64_t timestamp)
    : timestamp(timestamp), type(type) {
    switch (this->type) {
        case STRING:
            this->data = data;
            break;
        default:
            break;
    }
}
Object::Object(ObjectType type, int64_t timestamp) : timestamp(timestamp), type(type){
    if(this->type == HASH_TABLE){
        this->data=std::set<std::string> {};
    }
}
auto Object::asString() const -> std::optional<std::string> {
    if (auto ptr = std::get_if<std::string>(&data)) {
        return *ptr;
    }
    return std::nullopt;
}

auto Object::asSet() const -> std::optional<std::set<std::string> >{
    if (auto ptr = std::get_if<std::set<std::string> >(&data)) {
        return *ptr;
    }
    return std::nullopt;
}
auto Object::get_timestamp() const -> int64_t { return timestamp; }

auto Object::set_timestamp(int64_t timestamp) -> void {
    this->timestamp = timestamp;
}

auto Object::get_current_time() -> int64_t {
    return duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

auto Object::encode() const -> std::string {
    std::ostringstream oss;
    switch (this->type) {
        case STRING:
            oss << "STRING" << " " << std::get<std::string>(this->data);
            return oss.str();
        case HASH_TABLE:
            oss << "SET {";
            for(auto entry: std::get<std::set<std::string> >(this->data))
               oss << " " << entry << " ";
            oss<<" } ";
            return  oss.str(); 
    }
}
