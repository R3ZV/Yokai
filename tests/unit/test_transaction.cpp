#include "../../daemon/include/database.h"
#include "../../daemon/include/list_database.h"
#include "../../daemon/include/transaction.h"
#include "../../lib/doctest.h"

TEST_CASE("Testing multiple transactions") {
    ListDatabase* global_database = ListDatabase::get_instance();
    Transaction* transaction_1 = new Transaction(global_database);
    Transaction* transaction_2 = new Transaction(global_database);

    // T1
    transaction_1->handle_command("MULTI");
    transaction_1->handle_command("SET radu 30");
    transaction_1->handle_command("SET razvan 50");
    transaction_1->handle_command("EXEC");

    // T2
    transaction_2->handle_command("MULTI");
    transaction_2->handle_command("SET radu 40");
    transaction_2->handle_command("SET razvan 40");
    transaction_2->handle_command("EXEC");

    transaction_1->handle_command("SHOW");

    CHECK(global_database->select_latest("radu", Object::get_current_time())
              .value()
              .get()
              ->asString() == "40");
    CHECK(global_database->select_latest("razvan", Object::get_current_time())
              .value()
              .get()
              ->asString() == "40");
}
TEST_CASE("Testing phantom reads inside of transactions") {
    ListDatabase* global_database = ListDatabase::get_instance();
    Transaction* transaction_1 = new Transaction(global_database);
    Transaction* transaction_2 = new Transaction(global_database);

    // Set "radu" to 30
    transaction_2->handle_command("SET radu 30");

    // T2 starts
    transaction_2->handle_command("MULTI");

    // T1 updates "radu" to 40 and commits
    transaction_1->handle_command("MULTI");
    transaction_1->handle_command("SET radu 40");
    transaction_1->handle_command("EXEC");

    transaction_1->handle_command("SHOW");

    // T2 reads key "radu" (should read 30)
    transaction_2->handle_command("SELECT radu");
    CHECK(transaction_2->get_local_store()
              ->select("radu")
              .value()
              .get()
              ->asString() == "30");
    transaction_2->handle_command("EXEC");
}
TEST_CASE("Testing conflicting transactions") {
    ListDatabase* global_database = ListDatabase::get_instance();
    Transaction* transaction_1 = new Transaction(global_database);
    Transaction* transaction_2 = new Transaction(global_database);

    // T2 starts
    transaction_2->handle_command("MULTI");

    // T1 updates "radu" to 40 and commits
    transaction_1->handle_command("MULTI");
    transaction_1->handle_command("SET radu 40");
    transaction_1->handle_command("EXEC");

    // T2 tries to also update key "radu"
    transaction_2->handle_command("SET radu 50");
    transaction_2->handle_command("EXEC");

    CHECK(global_database->select_latest("radu", Object::get_current_time())
              .value()
              .get()
              ->asString() == "40");
}
