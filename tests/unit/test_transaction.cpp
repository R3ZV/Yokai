#include "../../daemon/include/database.h"
#include "../../daemon/include/list_database.h"
#include "../../daemon/include/transaction.h"
#include "../../lib/doctest.h"

TEST_CASE("Testing multiple transactions") {
    ListDatabase* global_database = new ListDatabase();
    Transaction* transaction_1 = new Transaction(global_database);
    Transaction* transaction_2 = new Transaction(global_database);

    std::string blob1 =
        "MULTI\n"
        "SET radu 30\n"
        "SET razvan 50\n"
        "EXEC\n";

    transaction_1->handle_command(blob1);

    std::string blob2 =
        "MULTI\n"
        "SET radu 40\n"
        "SET razvan 40\n"
        "EXEC\n";

    transaction_2->handle_command(blob2);

    CHECK(global_database->select_latest("radu", Object::get_current_time())
              .value()
              .get()
              ->asString() == "40");
    CHECK(global_database->select_latest("razvan", Object::get_current_time())
              .value()
              .get()
              ->asString() == "40");
}

TEST_CASE("Testing conflicting transactions") {
    ListDatabase* global_database = new ListDatabase();
    Transaction* transaction_1 = new Transaction(global_database);
    Transaction* transaction_2 = new Transaction(global_database);

    std::string blob2 =
        "MULTI\n"
        "SET radu 30\n"
        "EXEC\n";


    // T2 starts
    transaction_2->handle_command("MULTI");

    std::string blob1 =
        "MULTI\n"
        "SET radu 40\n"
        "EXEC\n";


    // T1 updates "radu" to 40 and commits
    transaction_1->handle_command(blob1);

    // T2 tries to also update key "radu"
    CHECK(global_database->select_latest("radu", Object::get_current_time())
              .value()
              .get()
              ->asString() == "40");
}
