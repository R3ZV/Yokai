#include "include/list_database.h"

#include <cassert>
#include <iostream>
#include <print>
#include <mutex>

#include "include/object.h"

ListDatabase* ListDatabase::db_instance = nullptr;

ListDatabase::ListDatabase(){ }

auto ListDatabase::get_instance() -> ListDatabase* {
    static std::mutex instance_mutex;
    std::lock_guard<std::mutex> db_lock(instance_mutex);
    if (!db_instance) {
        db_instance = new ListDatabase();
    }
    return db_instance;
}
auto ListDatabase::select_latest(const std::string& key,
                                 int64_t transaction_timestamp)
    -> std::expected<std::shared_ptr<Object>, std::error_code> {
    // Check if the key exists in the map
    const auto& it = data.find(key);
    if (it != data.end()) {
        // We return the object with the latest timestamp that is EARLIER than
        // the current transaction's timestamp to avoid phantom reads
        const auto& object_list = it->second;
        for (auto object = object_list.rbegin(); object != object_list.rend();
             object++) {
            if (object->get()->get_timestamp() <= transaction_timestamp) {
                return *object;
            }
        }
    }
    // Return an error message if the key is not found
    return std::unexpected(std::error_code(errno, std::generic_category()));
}

auto ListDatabase::show_objects() -> void {
    if (this->data.empty()) {
        std::println(std::cerr, "\nNothing to show!");
        return;
    }
    std::println(std::cerr, "\nShowing data:");
    for (const auto& it : this->data) {
        println(std::cerr, "Key: {}", it.first);
        for (const auto& value : it.second) {
            auto aux = value.get()->asString();
            if (aux.has_value()) {
                auto printable_value = aux.value();
                println(std::cerr, "{} - {}", printable_value,
                        value.get()->get_timestamp());
            }
        }
    }
}

auto ListDatabase::update(Database& write_buffer, int64_t commit_timestamp)
    -> std::expected<void, std::error_code> {
    std::unique_lock<std::mutex> lock(this->commit_lock);
    try {
        // First check there are no collissions with other transaction commits
        for (const auto& it : write_buffer.get_data()) {
            const auto& [key, value] = it;
            // We check if there have been any commits for any key since our
            // transaction started
            if (this->exists(key) &&
                this->data[key].back().get()->get_timestamp() >
                    value.get()->get_timestamp()) {
                std::println("Aborting transaction!");
                return {};
            }
        }

        // Update the timestamps of the objects inside of the write buffer to
        // the commit timestamp
        write_buffer.update_timestamps(commit_timestamp);

        // Update global dict with new values
        for (const auto& it : write_buffer.get_data()) {
            this->data[it.first].push_back(it.second);
        }
    } catch (...) {
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }
    return {};
}

auto ListDatabase::exists(const std::string& key) -> bool {
    return (this->data.find(key) != this->data.end());
}

auto ListDatabase::get_data() const
    -> const std::map<std::string, std::deque<std::shared_ptr<Object>>>& {
    return this->data;
}

auto ListDatabase::load_from_file() -> std::expected<void, std::string> {
    // We grab the lock because method can be called by command ROLLBACK as well
    // So we treat it like a commit
    std::unique_lock<std::mutex> lock(this->commit_lock);

    const std::string filename = "saves/dump.txt";

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println(std::cerr,
                     "[DBG] Dump file not found, starting with empty dataset");
        return {};
    }

    // Clear the map in case the function is called with ROLLBACK
    data.clear();

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        ++line_num;

        std::istringstream iss(line);
        std::string key, type, value;
        iss >> key >> type >> value;
        if (key.length() == 0 && type.length() == 0 && value.length() == 0) {
            return std::unexpected(
                "[ERR]: Invalid database line format, one of the values is "
                "empty!");
        }

        try {
            if (type == "STRING") {
                data.emplace(key, std::deque<std::shared_ptr<Object>>{
                                      std::make_shared<Object>(
                                          ObjectType::STRING, value)});
            } else {
                return std::unexpected("[ERR] Unknown type '" + type +
                                       "' on line " + std::to_string(line_num));
            }
        } catch (const std::exception& e) {
            return std::unexpected("[ERR] Failed to parse value on line " +
                                   std::to_string(line_num) + ": " + e.what());
        }
    }

    return {};
}
