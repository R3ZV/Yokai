#include "include/database.h"
#include <map>
#include <string>
#include <print>

auto Database::insert_key(const std::string& key, std::shared_ptr<Object> value) -> std::optional<std::error_code> {
    try {
        data[key] = value;
    }
    catch (...) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }    
    return std::nullopt;
}

auto Database::delete_key(const std::string& key) -> std::optional<std::error_code> {
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

auto Database::select(const std::string& key) -> std::expected<std::shared_ptr<Object>, std::error_code> {
    // Check if the key exists in the map
    const auto& it = data.find(key);
    if (it != data.end()) {
        return std::expected<std::shared_ptr<Object>, std::error_code>(it->second);
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
        auto aux = it.second.get()->asString();
        if (aux.has_value()) {
            auto printable_value = aux.value();
            println("Key: {}, Value: {}, Timestamp: {}", it.first, printable_value, it.second.get()->get_timestamp());
        } 
    }
}

void Database::clear() {
    this->data.clear();
}

bool Database::exists(const std::string& key) const {
    return (this->data.find(key) != this->data.end());
}

auto Database::get_data() const -> const std::map<std::string, std::shared_ptr<Object>>& {
    return this->data;
}

void Database::update_timestamps(time_t timestamp) {
    for (const auto& it : this->data) {
        it.second.get()->set_timestamp(timestamp);
    }
}