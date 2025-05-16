#pragma once
#include <vector>

#include "command.h"
#include "database.h"
#include "list_database.h"

class Transaction {
   private:
    ListDatabase* global_store;
    Database* local_store;
    Database* write_buffer;
    std::vector<Command> commands;
    bool ongoing = false;
    int64_t timestamp;

   public:
    Transaction(ListDatabase* global_store);
    auto handle_command(const std::string& buff) -> void;
    auto commit() -> void;
    ~Transaction();
    auto get_local_store() -> Database*;
};
