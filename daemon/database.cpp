#include "include/database.h"

#include <iostream>
#include <map>
#include <print>
#include <string>
#include <optional>
/**
 * This function acts as both an insert and an upload
 * since if you already have the key in the database
 * and call insert again it will replace the value
 * with the new one.
 *
 * @param key which entry in the database to change
 * @param value is the new value of key
 * @return an error in case setting the key to value failed or void
 */
auto Database::insert_key(const std::string& key, std::shared_ptr<Object> value)
    -> std::expected<void, std::error_code> {
    try {
        data[key] = value;
    } catch (...) {
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }
    return {};
}
auto Database::add_key_set(const std::string& key, std::shared_ptr<Object> value) 
    -> std::expected<void, std::error_code>  {
    try{
        bool has_set=(data[key]->asSet())!=std::nullopt;
        if(data.count(key) == 0 || (!has_set)){ //if the key is nonexistent or has an allotted string
            data[key]=std::make_shared<Object>(ObjectType::HASH_TABLE);
        }
        //if we get here, there has to be a set
        std::optional<std::string> string_opt=value->asString();
        if(string_opt.has_value()){
            std::string string_value=string_opt.value();
            data[key]->asSet()->insert(string_value); 
        }
    }
    catch(...){
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
    // TODO: Return an error which specifies that there is no entry with value
    // `key`
    return std::unexpected(std::error_code(errno, std::generic_category()));
}

auto Database::show_objects() -> void {
    if (this->data.empty()) {
        std::println(std::cerr, "\nNothing to show!");
        return;
    }
    std::println(std::cerr, "\nShowing data:");
    for (const auto& it : this->data) {
        auto aux = it.second.get()->asString();
        if (aux.has_value()) {
            auto printable_value = aux.value();
            println(std::cerr, "Key: {}, Value: {}, Timestamp: {}", it.first,
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

auto Database::update_timestamps(int64_t timestamp) -> void {
    for (const auto& it : this->data) {
        it.second.get()->set_timestamp(timestamp);
    }
}
