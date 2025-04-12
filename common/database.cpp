#include "include/database.h"
#include <map>
#include <string>
#include <print>

void Database::insert(std::string key, const Object& value) {
    data[key] = value;
}

std::expected<Object, std::error_code> Database::select(std::string key) {
    // Check if the key exists in the map
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    } else {
        // Return an error message if the key is not found
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }
}

void Database::show_objects() {
    std::println("\nShowing data:");
    for (const auto& it : this->data) {
        std::println("{}", it.first);
    }
}
