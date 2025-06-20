#include "include/transaction.h"

#include <iostream>
#include <print>

#include "include/command.h"

Transaction::Transaction(ListDatabase* global_store)
    : global_store(global_store),
      local_store(new Database()),
      write_buffer(new Database()),
      timestamp(Object::get_current_time()) {}

auto Transaction::handle_command(const std::string& buff) -> std::string {
    std::vector<Command> commands = Command::parse(buff);

    if (this->ongoing) {
        // for debug
        for (Command cmd : commands) this->commands.push_back(cmd);
    }

    // Update the transaction timestamp if it is a new transaction
    if (!this->ongoing) {
        this->timestamp = Object::get_current_time();
    }

    std::string response;
    for (Command cmd : commands) {
        // One line command
        switch (cmd.get_type()) {
            case CommandType::MULTI: {
                this->ongoing = true;
            } break;

            case CommandType::EXEC: {
                this->ongoing = false;
            } break;

            case CommandType::SET: {
                auto args = cmd.get_args();
                std::string key = args[0];
                auto val = args[1];
                std::println(std::cerr, "[DBG]: Setting key {} to value {}",
                             key, val);

                auto new_value = std::make_shared<Object>(ObjectType::STRING,
                                                          val, this->timestamp);

                // This is a write, so we change the value in the
                // write_buffer
                this->write_buffer->insert_key(key, new_value);

                // And also in the local store
                this->local_store->insert_key(key, new_value);
                response += "Value successfully inserted!\n";
            } break;
            case CommandType::SADD: {
                auto args = cmd.get_args();
                std::string key = args[0];
                auto val = args[1];

                auto hash_set =
                    this->global_store->select_latest(key, this->timestamp);
                if (hash_set.has_value()) {
                    std::shared_ptr<Object> copy =
                        std::make_shared<Object>(*hash_set.value());
                    std::println(std::cerr,
                                 "[DBG]: Setting key {} to value {} into set",
                                 key, val);
                    this->local_store->insert_into_set(copy, key, val);
                    this->write_buffer->insert_into_set(copy, key, val);
                }

            } break;
            case CommandType::DEL: {
                auto args = cmd.get_args();
                std::string key = args[0];
                if (this->local_store->exists(key)) {
                    // Delete the key
                    auto res = this->local_store->delete_key(key);
                    if (!res.has_value()) {
                        std::println(std::cerr, "{}", res.error().message());
                        return res.error().message();
                    }
                }
                if (this->write_buffer->exists(key)) {
                    // Delete the key
                    auto res = this->write_buffer->delete_key(key);
                    if (!res.has_value()) {
                        std::println(std::cerr, "{}", res.error().message());
                        return res.error().message();
                    }
                }

                // Look for the key in global store
                // We look for an object that is visible from inside the
                // current transaction (not a later one)
                if (this->global_store->select_latest(key, this->timestamp)
                        .has_value()) {
                    // Mark the key as deleted
                    auto res = this->write_buffer->insert_key(
                        key,
                        std::make_shared<Object>(ObjectType::STRING, "DELETED",
                                                 this->timestamp));
                    if (!res.has_value()) {
                        std::println(std::cerr, "{}", res.error().message());
                        return res.error().message();
                    }
                    response += "Key successfully deleted!\n";
                }
            } break;

            case CommandType::SELECT: {
                auto args = cmd.get_args();
                std::string key = args[0];

                // First look for the key in the local store
                auto res = this->local_store->select(key);

                if (!res.has_value()) {
                    // Then look in the global store
                    res =
                        this->global_store->select_latest(key, this->timestamp);

                    if (!res.has_value()) {
                        std::println(std::cerr, "{}", res.error().message());
                        return res.error().message();
                    }

                    // Add the object to the local store
                    const auto& value = res.value();
                    this->local_store->insert_key(key, value);

                    if (!this->ongoing) {
                        response += "\n";
                    }
                }

                // Print the key value pair
                const auto& value = res.value();
                auto aux = value.get()->asString();
                if (aux.has_value()) {
                    auto printable_value = aux.value();
                    println(std::cerr, "Key: {}, Value: {}", key,
                            printable_value);

                    response += std::format("Key: {}, Value: {}\n", key,
                                            printable_value);
                }
            } break;

            case CommandType::SHOW: {
                this->global_store->show_objects();
            } break;

            case CommandType::SHOW_LOCAL: {
                this->local_store->show_objects();
            } break;

            case CommandType::SHOW_WRITE: {
                this->write_buffer->show_objects();
            } break;

            case CommandType::ROLLBACK: {
                this->rollback = true;
                this->ongoing = false;
            }

            case CommandType::INVALID: {
                this->ongoing = false;
            } break;
        }
    }

    // Commit changes only after all commands have been processed
    if (this->ongoing == false) {
        this->commit();
        commands.clear();
    }

    return response;
}

auto Transaction::commit() -> void {
    if (!commands.empty()) {
        std::println(std::cerr, "[DBG] Commands exectued: ");
        for (Command command : this->commands) {
            std::println(std::cerr, "[T]: {} ", command);
        }
    }

    // Update the global dict with the local changes
    int64_t commit_time = Object::get_current_time();
    if (this->rollback == true) {
        std::println(std::cerr, "[DBG] Rolling back...");
        global_store->load_from_file();
        this->rollback = false;
    } else {
        global_store->update(*write_buffer, commit_time);
    }
    commands.clear();
    local_store->clear();
    write_buffer->clear();
}

Transaction::~Transaction() {
    delete local_store;
    delete write_buffer;
}

auto Transaction::get_local_store() -> Database* { return this->local_store; }
