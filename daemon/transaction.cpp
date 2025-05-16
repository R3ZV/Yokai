#include "include/transaction.h"

#include <iostream>
#include <print>

#include "include/command.h"

Transaction::Transaction(ListDatabase* global_store)
    : global_store(global_store),
      local_store(new Database()),
      write_buffer(new Database()),
      timestamp(Object::get_current_time()) {}

auto Transaction::handle_command(const std::string& buff) -> void {
    // TODO:
    // based on the current implementation the buffer will always
    // contain a single command so we will keep the
    // std::vector<Command> for now until further discussions
    std::vector<Command> commands = Command::parse(buff);

    if (this->ongoing) {
        this->commands.push_back(commands[0]);
    }

    // Update the transaction timestamp if it is a new transaction
    if (!this->ongoing) {
        this->timestamp = Object::get_current_time();
    }

    if (!commands.empty()) {
        // One line command
        switch (commands[0].get_type()) {
            case CommandType::SET: {
                auto args = commands[0].get_args();
                std::println("[DBG]: Setting key {} to value {}", args[0],
                             args[1]);

                auto new_value =
                    std::make_shared<Object>(args[1], this->timestamp);

                // This is a write, so we change the value in the
                // write_buffer
                this->write_buffer->insert_key(args[0], new_value);

                // And also in the local store
                this->local_store->insert_key(args[0], new_value);
            } break;

            case CommandType::DEL: {
                auto args = commands[0].get_args();
                if (this->local_store->exists(args[0])) {
                    // Delete the key
                    auto res = this->local_store->delete_key(args[0]);
                    if (!res.has_value()) {
                        std::cerr << res.error().message() << std::endl;
                        return;
                    }
                }
                if (this->write_buffer->exists(args[0])) {
                    // Delete the key
                    auto res = this->write_buffer->delete_key(args[0]);
                    if (!res.has_value()) {
                        std::cerr << res.error().message() << std::endl;
                        return;
                    }
                }

                // Look for the key in global store
                // We look for an object that is visible from inside the
                // current transaction (not a later one)
                if (this->global_store->select_latest(args[0], this->timestamp)
                        .has_value()) {
                    // Mark the key as deleted
                    auto res = this->write_buffer->insert_key(
                        args[0],
                        std::make_shared<Object>("DELETED", this->timestamp));
                    if (!res.has_value()) {
                        std::cerr << res.error().message() << std::endl;
                        return;
                    }
                }
            } break;

            case CommandType::SELECT: {
                auto args = commands[0].get_args();
                // First look for the key in the local store
                auto res = this->local_store->select(args[0]);

                if (!res.has_value()) {
                    // Then look in the global store
                    res = this->global_store->select_latest(args[0],
                                                            this->timestamp);

                    if (!res.has_value()) {
                        std::cerr << res.error().message() << std::endl;
                        return;
                    }

                    // Add the object to the local store
                    std::println("Debug");
                    const auto& value = res.value();
                    this->local_store->insert_key(args[0], value);
                }

                // Print the key value pair
                const auto& value = res.value();
                auto aux = value.get()->asString();
                if (aux.has_value()) {
                    auto printable_value = aux.value();
                    println("Key: {}, Value: {}", args[0], printable_value);
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

            case CommandType::MULTI: {
                this->ongoing = true;
            } break;

            case CommandType::EXEC: {
                this->ongoing = false;
            } break;

            case CommandType::INVALID: {
                // TODO: find a way to send this error
                // to the user, maybe handle_command should now
                // return a message
                this->ongoing = false;
            } break;
        }
    }

    // Commit changes only after all commands have been processed
    if (this->ongoing == false) {
        this->commit();
        commands.clear();
    }
}

auto Transaction::commit() -> void {
    if (!commands.empty()) {
        std::println("[DBG] Commands exectued: ");
        for (Command command : this->commands) {
            std::println("[T]: {} ", command);
        }
    }

    // Update the global dict with the local changes
    int64_t commit_time = Object::get_current_time();
    global_store->update(*write_buffer, commit_time);
    commands.clear();
    local_store->clear();
    write_buffer->clear();
}

Transaction::~Transaction() {
    delete local_store;
    delete write_buffer;
}

auto Transaction::get_local_store() -> Database* { return this->local_store; }
