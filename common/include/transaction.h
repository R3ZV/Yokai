#pragma once
#include "database.h"
#include <vector>

class Transaction {
    private:
        Database* global_store;
        //Database* local_store;
        std::vector<std::vector<std::string>> commands;

    public:
        Transaction(Database* global_store);
        void add_command(char* buff);
        void commit();
};