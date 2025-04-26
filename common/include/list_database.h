#pragma once

#include "object.h"
#include "database.h"
#include<map>
#include<string>
#include <expected>
#include <system_error>
#include <deque>
#include <mutex>

class ListDatabase {
    private:
        std::map<std::string, std::deque<std::shared_ptr<Object>>> data;
        std::mutex commit_lock;
    public:
        auto select_latest(const std::string& key, time_t transaction_timestamp) 
            -> std::expected<std::shared_ptr<Object>, std::error_code>;
        void show_objects();
        auto update(Database& other, time_t commit_timestamp) -> std::optional<std::error_code>;
        bool exists(const std::string& key);
};