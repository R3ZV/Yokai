#pragma once

#include <expected>
#include <map>
#include <memory>
#include <string>
#include <system_error>

#include "object.h"

class Database {
    std::map<std::string, std::shared_ptr<Object>> data;

   public:
    auto insert_key(const std::string& key, std::shared_ptr<Object> value)
        -> std::expected<void, std::error_code>;
    auto add_key_set(const std::string& key, std::shared_ptr<Object> value) 
    -> std::expected<void, std::error_code>;
    auto delete_key(const std::string& key)
        -> std::expected<void, std::error_code>;
    auto select(const std::string& key)
        -> std::expected<std::shared_ptr<Object>, std::error_code>;

    auto show_objects() -> void;
    auto clear() -> void;
    auto exists(const std::string& key) const -> bool;
    auto get_data() const
        -> const std::map<std::string, std::shared_ptr<Object>>&;
    auto update_timestamps(int64_t timestamp) -> void;
};
