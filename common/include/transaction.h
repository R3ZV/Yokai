#pragma once
#include "database.h"
#include <vector>

class Transaction {
    private:
        Database* global_store;
        Database* local_store;
        Database* write_buffer;
        std::vector<std::vector<std::string>> commands;
        bool ongoing = false;
        time_t timestamp;

    public:
        Transaction(Database* global_store);
        void handle_command(char* buff);
        void commit();
        ~Transaction();
};