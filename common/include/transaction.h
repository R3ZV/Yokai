#pragma once
#include "database.h"
#include <vector>

class Transaction {
    private:
        Database* global_store;
        //Database* local_store;
        std::vector<std::vector<std::string>> commands;
        bool ongoing = false;

    public:
        Transaction(Database* global_store);
        void handle_command(char* buff);
        void commit();
};