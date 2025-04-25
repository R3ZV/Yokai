#include "include/list_database.h"
#include <print>

std::expected<Object, std::error_code> ListDatabase::select_latest(const std::string& key, time_t transaction_timestamp) {
    // Check if the key exists in the map
    const auto& it = data.find(key);
    if (it != data.end()) {
        // We return the object with the latest timestamp that is EARLIER than the current transaction's timestamp
        // to avoid phantom reads
        const auto& object_list = it->second;
        for (auto object = object_list.rbegin(); object != object_list.rend(); object++) {
            if (object->get_timestamp() < transaction_timestamp) {
                return std::expected<Object, std::error_code>(*object);
            }
        }        
    }
    // Return an error message if the key is not found
    return std::unexpected(std::error_code(errno, std::generic_category()));
}

void ListDatabase::show_objects() {
    if (this->data.empty()) {
        std::println("\nNothing to show!");
        return;
    }
    std::println("\nShowing data:");
    for (const auto& it : this->data) {
        println("Key: {}", it.first);
        for (auto value : it.second) {
            auto aux = value.asString();
            if (aux.has_value()) {
                auto printable_value = aux.value();
                println("{} - {}", printable_value, value.get_timestamp());
            }
        }         
    }
}

std::optional<std::error_code> ListDatabase::update(const Database& other) {
    try {    
        for (const auto& it : other.get_data()) {
            this->data[it.first].push_back(it.second);
        }
    }
    catch (...) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }    
    return std::nullopt;  
}

bool ListDatabase::exists(const std::string& key) {
    return (this->data.find(key) != this->data.end());
}