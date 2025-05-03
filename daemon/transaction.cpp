#include "include/transaction.h"

#include <string.h>
#include <sstream>
#include <iostream>
#include <print>

Transaction::Transaction(ListDatabase* global_store)
    : global_store(global_store),
      local_store(new Database()),
      write_buffer(new Database()),
      timestamp(Object::get_current_time()) {}

auto Transaction::handle_command(std::string buff) -> void {
    // COMMANDS:
    // SET [key] [val]
    // DEL [key]
    // SELECT [key]
    // SHOW  (shows global dict)
    // SHOW LOCAL  (shows thread local dict/read buffer)
    // SHOW WRITE  (shows write buffer)
    // MULTI  (begin multiple command transaction)
    // EXEC  (execute multiple command transaction)
    std::vector<std::string> tokens;
    std::istringstream iss(buff);
    std::string token;

    while (iss >> token) {
        // Add token to vector
        tokens.push_back(token);
    }

    std::println("[BDG]: Tokens: ");
    for (const auto& t : tokens) {
        std::println("{}", t);
    }

    // Add command to command list if transaction contains multiple commands
    // (for debug)
    if (this->ongoing) {
        this->commands.push_back(tokens);
    }
    // Update the transaction timestamp if it is a new transaction
    if (!this->ongoing) {
        this->timestamp = Object::get_current_time();
    }

    // SET
    if (tokens.size() == 3 && tokens[0] == "SET") {
        std::println("[DBG]: Setting key {} to value {}", tokens[1], tokens[2]);
        std::shared_ptr<Object> new_value =
            std::make_shared<Object>(tokens[2], this->timestamp);
        // This is a write, so we change the value in the write_buffer
        this->write_buffer->insert_key(tokens[1], new_value);
        // And also in the local store
        this->local_store->insert_key(tokens[1], new_value);
    }
    // DEL
    else if (tokens.size() == 2 && tokens[0] == "DEL") {
        // First we clean up the key from the local store and write buffer
        if (this->local_store->exists(tokens[1])) {
            // Delete the key
            auto res = this->local_store->delete_key(tokens[1]);
            if (!res.has_value()) {
                std::cerr << res.error().message() << std::endl;
                return;
            }
        }
        if (this->write_buffer->exists(tokens[1])) {
            // Delete the key
            auto res = this->write_buffer->delete_key(tokens[1]);
            if (!res.has_value()) {
                std::cerr << res.error().message() << std::endl;
                return;
            }
        }

        // Look for the key in global store
        // We look for an object that is visible from inside the current
        // transaction (not a later one)
        if (this->global_store->select_latest(tokens[1], this->timestamp)
                .has_value()) {
            // Mark the key as deleted
            auto res = this->write_buffer->insert_key(
                tokens[1],
                std::make_shared<Object>("DELETED", this->timestamp));
            if (!res.has_value()) {
                std::cerr << res.error().message() << std::endl;
                return;
            }
        }

    }
    // SELECT
    else if (tokens.size() == 2 && tokens[0] == "SELECT") {
        // First look for the key in the local store
        auto res = this->local_store->select(tokens[1]);

        if (!res.has_value()) {
            // Then look in the global store
            res = this->global_store->select_latest(tokens[1], this->timestamp);
            if (!res.has_value()) {
                std::cerr << res.error().message() << std::endl;
                return;
            }
            // Add the object to the local store
            std::println("Debug");
            const auto& value = res.value();
            this->local_store->insert_key(tokens[1], value);
        }
        // Print the key value pair
        const auto& value = res.value();
        auto aux = value.get()->asString();
        if (aux.has_value()) {
            auto printable_value = aux.value();
            println("Key: {}, Value: {}", tokens[1], printable_value);
        }
    }
    // SHOW
    else if (tokens.size() == 1 && tokens[0] == "SHOW") {
        this->global_store->show_objects();
    }
    // SHOW LOCAL (for debugging purposes)
    else if (tokens.size() == 2 && tokens[0] == "SHOW" &&
             tokens[1] == "LOCAL") {
        this->local_store->show_objects();
    }
    // SHOW WRITE (for debug)
    else if (tokens.size() == 2 && tokens[0] == "SHOW" &&
             tokens[1] == "WRITE") {
        this->write_buffer->show_objects();
    }
    // MULTI
    else if (tokens.size() == 1 && tokens[0] == "MULTI") {
        this->ongoing = true;
    }
    // EXEC
    else if (tokens.size() == 1 && tokens[0] == "EXEC") {
        this->ongoing = false;
    } else {
        std::println("Not a recognised command");
    }

    // Commit changes only after all commands have been processed
    if (this->ongoing == false) {
        this->commit();
        commands.clear();
    }
}

auto Transaction::commit() -> void {
    // Print all the commands inside the transaction
    if (commands.size()) {
        std::println("[DBG] Commands exectued: ");
        for (auto command : commands) {
            std::print("Command: ");
            for (auto token : command) {
                std::print("{} ", token);
            }
            std::println();
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

auto Transaction::get_local_store() -> Database* {
    return this->local_store;
}