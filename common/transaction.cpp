#include "include/transaction.h"
#include <string.h>
#include <print>
#include <iostream>

Transaction::Transaction(Database* global_store) : global_store(global_store) {}

void Transaction::add_command(char* buff) {
        // check if the message is SET smth or DELETE smth
        // also SHOW to show entire database
        // also MULTI and EXEC for multi command transactions
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

        this->commands.push_back(tokens);
        
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
        // SHOW DATA
        else if (tokens.size() == 1 && tokens[0] == "SHOW") {
            this->global_store->show_objects();
        }
        // MULTI
        else if (tokens.size() == 1 && tokens[0] == "MULTI") {
        }
        else {
            std::println("Not a recognised command");
        }
}

void Transaction::commit() {

}