#include "include/transaction.h"
#include <string.h>
#include <print>
#include <iostream>

Transaction::Transaction(Database* global_store) : global_store(global_store) {}

void Transaction::handle_command(char* buff) {
        // check if the message is SET smth or DELETE smth
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
        
        // SET
        if (tokens.size() == 3 && tokens[0] == "SET") {            
            std::println("[DBG]: Setting key {} to value {}", tokens[1], tokens[2]);  
            this->global_store->insert_key(tokens[1], Object(tokens[2]));        
        }
        // DELETE
        else if (tokens.size() == 2 && tokens[0] == "DEL") {
            std::println("[DBG]: Deleting key {}", tokens[1]);
            auto res = this->global_store->delete_key(tokens[1]);
            if(res != std::nullopt) {
                std::cerr << res.value().message() << std::endl;
            }
        }
        // SELECT (for now just prints the key value pair)
        else if (tokens.size() == 2 && tokens[0] == "SELECT") {
            auto res = this->global_store->select(tokens[1]);
            if (!res.has_value()) {
                std::cerr << res.error().message() << std::endl;
                return;
            }
            const auto& value = res.value();
            auto aux = value.asString();
            if (aux.has_value()) {
                auto printable_value = aux.value();
                println("Key: {}, Value: {}", tokens[1], printable_value);
            }            
        }
        // SHOW DATA
        else if (tokens.size() == 1 && tokens[0] == "SHOW") {
            this->global_store->show_objects();
        }
        // MULTI
        else if (tokens.size() == 1 && tokens[0] == "MULTI") {
            this->ongoing = true;
        }
        // EXEC
        else if (tokens.size() == 1 && tokens[0] == "EXEC") {
            this->ongoing = false;
            this->commit();
        }
        else {
            std::println("Not a recognised command");
        }
}

void Transaction::commit() {
    for (auto command : commands) {
        std::print("Command: ");
        for (auto token : command) {
            std::print("{} ", token);
        }
        std::println();
    }
    commands.clear();
}