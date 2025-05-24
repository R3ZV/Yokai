#pragma once

#include <deque>
#include <expected>
#include <map>
#include <mutex>
#include <string>
#include <system_error>

#include "database.h"
#include "object.h"

class ListDatabase {
   private:
    std::map<std::string, std::deque<std::shared_ptr<Object>>> data;
    std::mutex commit_lock;

   public:
    auto select_latest(const std::string& key, int64_t transaction_timestamp)
        -> std::expected<std::shared_ptr<Object>, std::error_code>;
    auto show_objects() -> void;
    auto update(Database& other, int64_t commit_timestamp)
        -> std::expected<void, std::error_code>;
    auto exists(const std::string& key) -> bool;
    auto get_data() const
        -> const std::map<std::string, std::deque<std::shared_ptr<Object>>>&;
};
