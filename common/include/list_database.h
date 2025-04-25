#pragma once

#include "object.h"
#include "database.h"
#include<map>
#include<string>
#include <expected>
#include <system_error>
#include <deque>

class ListDatabase {
    private:
        std::map<std::string, std::deque<Object>> data;
    public:
        std::expected<Object, std::error_code> select_latest(const std::string& key, time_t transaction_timestamp);
        void show_objects();
        std::optional<std::error_code> update(const Database& other);
        bool exists(const std::string& key);
};