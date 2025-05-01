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
        -> std::optional<std::error_code>;
    auto delete_key(const std::string& key) -> std::optional<std::error_code>;
    auto select(const std::string& key)
        -> std::expected<std::shared_ptr<Object>, std::error_code>;
    void show_objects();
    void clear();
    bool exists(const std::string& key) const;
    auto get_data() const
        -> const std::map<std::string, std::shared_ptr<Object>>&;
    void update_timestamps(time_t timestamp);
};