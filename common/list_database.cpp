#include "include/list_database.h"
#include <print>

auto ListDatabase::select_latest(const std::string& key, time_t transaction_timestamp) 
    -> std::expected<std::shared_ptr<Object>, std::error_code> {
    // Check if the key exists in the map
    const auto& it = data.find(key);
    if (it != data.end()) {
        // We return the object with the latest timestamp that is EARLIER than the current transaction's timestamp
        // to avoid phantom reads
        const auto& object_list = it->second;
        for (auto object = object_list.rbegin(); object != object_list.rend(); object++) {
            if (object->get()->get_timestamp() < transaction_timestamp) {
                return std::expected<std::shared_ptr<Object>, std::error_code>(*object);
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
        for (const auto& value : it.second) {
            auto aux = value.get()->asString();
            if (aux.has_value()) {
                auto printable_value = aux.value();
                println("{} - {}", printable_value, value.get()->get_timestamp());
            }
        }         
    }
}

auto ListDatabase::update(Database& write_buffer, time_t commit_timestamp) -> std::optional<std::error_code> {
    std::unique_lock<std::mutex> lock(this->commit_lock);    
    try {
        // First check there are no collissions with other transaction commits
        for (const auto& it : write_buffer.get_data()) {
            const auto& [key, value] = it;
            // We check if there have been any commits for any key since our transaction started
            if (this->exists(key) && this->data[key].back().get()->get_timestamp() > value.get()->get_timestamp()) {
                std::println("Aborting transaction!");
                return std::nullopt;
            }
        }           
        
        // Update the timestamps of the objects inside of the write buffer to the commit timestamp
        write_buffer.update_timestamps(commit_timestamp);

        // Update global dict with new values
        for (const auto& it : write_buffer.get_data()) {
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