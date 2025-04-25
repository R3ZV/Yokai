#include "include/transaction.h"
#include <string.h>
#include <print>
#include <iostream>

Transaction::Transaction(ListDatabase* global_store) : global_store(global_store), local_store(new Database()), 
        write_buffer(new Database()), timestamp(time(nullptr)) {}

void Transaction::handle_command(char* buff) {
        // check if the message is SET ... or DELETE ...
        // SELECT prints the value of the key
        // also SHOW to show entire database
        // also MULTI and EXEC for beggining and ending transactions
        std::vector<std::string> tokens;
        char* token = strtok(buff, " ");

        while (token != nullptr) {
            // Add token to vector
            tokens.push_back(token);
    
            // Get the next token
            token = strtok(nullptr, " ");
        }
        
        std::println("[BDG]: Tokens: ");
        for (const auto& t : tokens) {
            std::println("{}", t);
        }

        // add command to command list if transaction contains multiple commands
        if (this->ongoing) {
            this->commands.push_back(tokens);
        }
        // update the transaction timestamp if it is a new transaction
        if (!this->ongoing) {
            this->timestamp = time(nullptr);
        }
        
        // SET
        if (tokens.size() == 3 && tokens[0] == "SET") {            
            std::println("[DBG]: Setting key {} to value {}", tokens[1], tokens[2]);  
            Object new_value = Object(tokens[2], this->timestamp);
            // This is a write, so we change the value in the write_buffer
            this->write_buffer->insert_key(tokens[1], new_value); 
            // And also in the local store
            this->local_store->insert_key(tokens[1], new_value);       
        }
        // DELETE
        else if (tokens.size() == 2 && tokens[0] == "DEL") {
            // Look for the key in global store
            // We look for an object that is visible from inside the current transaction (not a later one)
            if (this->global_store->select_latest(tokens[1], this->timestamp).has_value()) {
                // Mark the key as deleted
                auto err = this->write_buffer->insert_key(tokens[1], Object("DELETED", this->timestamp));
                if (err != std::nullopt) {
                    std::cerr << err.value().message() << std::endl;
                    return;
                }
            }
            // Look for the key in local store
            if (this->local_store->exists(tokens[1])) {
                // Delete the key
                auto err = this->local_store->delete_key(tokens[1]);
                if (err != std::nullopt) {
                    std::cerr << err.value().message() << std::endl;
                    return;
                }
            }
        }
        // SELECT
        else if (tokens.size() == 2 && tokens[0] == "SELECT") {
            // First look for the key in local store
            auto res = this->local_store->select(tokens[1]);

            if (!res.has_value()) {
                // Then look in the global store
                res = this->global_store->select_latest(tokens[1], this->timestamp);
                if (!res.has_value()) {
                    std::cerr << res.error().message() << std::endl;
                    return;
                }
                // Add a copy of the object to local store (shouldnt really be a copy, whatever)
                std::println("Debug");
                const auto& value = res.value();
                this->local_store->insert_key(tokens[1], value);
            }
            // Print the key value pair
            const auto& value = res.value();
            auto aux = value.asString();
            if (aux.has_value()) {
                auto printable_value = aux.value();
                println("Key: {}, Value: {}", tokens[1], printable_value);
            }                                    
        }
        // SHOW DATA (from global storage)
        else if (tokens.size() == 1 && tokens[0] == "SHOW") {
            this->global_store->show_objects();
        }
        // SHOW LOCAL DATA (for debugging purposes)
        else if (tokens.size() == 2 && tokens[0] == "SHOW" && tokens[1] == "LOCAL") {
            this->local_store->show_objects();
        }  
        // SHOW WRITE BUFFER DATA (for debug)
        else if (tokens.size() == 2 && tokens[0] == "SHOW" && tokens[1] == "WRITE") {
            this->write_buffer->show_objects();
        }     
        // MULTI
        else if (tokens.size() == 1 && tokens[0] == "MULTI") {
            this->ongoing = true;
        }
        // EXEC
        else if (tokens.size() == 1 && tokens[0] == "EXEC") {
            this->ongoing = false;
        }
        else {
            std::println("Not a recognised command");
        }

        // Commit changes only after all commands have been processed
        if (this->ongoing == false) {
            this->commit();
            commands.clear();
        }
}

void Transaction::commit() {
    // print all the commands inside the transaction
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

    // update the global dict with the local changes
    global_store->update(*write_buffer);
    commands.clear();
    local_store->clear();
    write_buffer->clear();
}

Transaction::~Transaction() {
    delete local_store;
}