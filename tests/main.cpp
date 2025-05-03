#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <thread>

#include "../daemon/include/database.h"
#include "../daemon/include/list_database.h"
#include "../daemon/include/transaction.h"
#include "../lib/doctest.h"

TEST_CASE("main") {}
TEST_CASE("Testing Database insert_key") {
    Database* database = new Database();
    database->insert_key("test_1", std::make_shared<Object>(10));
    database->insert_key("test_2", std::make_shared<Object>("radu"));

    const std::map<std::string, std::shared_ptr<Object>>& data =
        database->get_data();

    CHECK(data.find("test_1") != data.end());
    CHECK(data.find("test_1")->second.get()->asInt() == 10);
    CHECK(data.find("test_2") != data.end());
    CHECK(data.find("test_2")->second.get()->asString() == "radu");
}
TEST_CASE("Testing Database select") {
    Database* database = new Database();
    database->insert_key("test_1", std::make_shared<Object>(10));
    database->insert_key("test_2", std::make_shared<Object>("radu"));

    CHECK(database->select("test_1").value().get()->asInt() == 10);
    CHECK(database->select("test_2").value().get()->asString() == "radu");
    CHECK(database->select("test_3").has_value() == false);
}
TEST_CASE("Testing Database delete_key") {
    Database* database = new Database();
    database->insert_key("test_1", std::make_shared<Object>(10));
    database->insert_key("test_2", std::make_shared<Object>("radu"));

    const std::map<std::string, std::shared_ptr<Object>>& data =
        database->get_data();

    CHECK(database->delete_key("test_1").has_value() == true);
    CHECK(database->delete_key("test_2").has_value() == true);
    CHECK(database->delete_key("test_3").has_value() == false);
    CHECK(data.empty());
}
TEST_CASE("Testing multiple transactions") {
    ListDatabase* global_database = new ListDatabase();
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
    ListDatabase* global_database = new ListDatabase();
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
    ListDatabase* global_database = new ListDatabase();
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