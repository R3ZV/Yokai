#include "include/database.h"

#include <map>
#include <print>
#include <string>

auto Database::insert_key(const std::string& key, std::shared_ptr<Object> value)
    -> std::expected<void, std::error_code> {
    try {
        data[key] = value;
    } catch (...) {
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }
    return {};
}

auto Database::delete_key(const std::string& key)
    -> std::expected<void, std::error_code> {
    auto it = data.find(key);
    if (it != data.end()) {
        data.erase(key);
        return {};
    }

    // TODO: should return a useful message
    return std::unexpected(std::error_code(errno, std::generic_category()));
}

auto Database::select(const std::string& key)
    -> std::expected<std::shared_ptr<Object>, std::error_code> {
    const auto& it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return std::unexpected(std::error_code(errno, std::generic_category()));
}

auto Database::show_objects() -> void {
    if (this->data.empty()) {
        std::println("\nNothing to show!");
        return;
    }
    std::println("\nShowing data:");
    for (const auto& it : this->data) {
        auto aux = it.second.get()->asString();
        if (aux.has_value()) {
            auto printable_value = aux.value();
            println("Key: {}, Value: {}, Timestamp: {}", it.first,
                    printable_value, it.second.get()->get_timestamp());
        }
    }
}

auto Database::clear() -> void { this->data.clear(); }

auto Database::exists(const std::string& key) const -> bool {
    return (this->data.find(key) != this->data.end());
}

auto Database::get_data() const
    -> const std::map<std::string, std::shared_ptr<Object>>& {
    return this->data;
}

auto Database::update_timestamps(time_t timestamp) -> void {
    for (const auto& it : this->data) {
        it.second.get()->set_timestamp(timestamp);
    }
}
