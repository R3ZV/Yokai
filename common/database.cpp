#include "include/database.h"
#include <map>
#include <string>
#include <print>

std::optional<std::error_code> Database::insert_key(const std::string& key, const Object& value) {
    try {
        data[key] = value;
    }
    catch (...) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }    
    return std::nullopt;
}

std::optional<std::error_code> Database::delete_key(const std::string& key) {
    // Check if the key exists in the map
    auto it = data.find(key);
    if (it != data.end()) {
        data.erase(key);
        return std::nullopt;
    } else {
        // TODO: should return a useful message
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }
}

std::expected<Object, std::error_code> Database::select(const std::string& key) {
    // Check if the key exists in the map
    const auto& it = data.find(key);
    if (it != data.end()) {
        return std::expected<Object, std::error_code>(it->second);
    } else {
        // Return an error message if the key is not found
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }
}

void Database::show_objects() {
    if (this->data.empty()) {
        std::println("\nNothing to show!");
        return;
    }
    std::println("\nShowing data:");
    for (const auto& it : this->data) {
        auto aux = it.second.asString();
        if (aux.has_value()) {
            auto printable_value = aux.value();
            println("Key: {}, Value: {}, Timestamp: {}", it.first, printable_value, it.second.get_timestamp());
        } 
    }
}

std::optional<std::error_code> Database::update(const Database& other) {
    try {
        for (const auto& it : other.data) {
            this->data[it.first] = it.second;
        }
    }
    catch (...) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }    
    return std::nullopt;    
}

void Database::clear() {
    this->data.clear();
}

bool Database::exists(const std::string& key) {
    return (this->data.find(key) != this->data.end());
}
