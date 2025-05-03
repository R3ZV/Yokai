#include "../daemon/include/command.h"
#include "../lib/doctest.h"

TEST_CASE("Test non-transactions") {
    std::string blob =
        "SET RADU 10252\n"
        "SET RAZVAN vac\n"
        "SELECT RAZVAN\n"
        "DEL RADU\n"
        "SHOW\n"
        "SHOW LOCAL\n"
        "SHOW WRITE\n";

    auto res = Command::parse(blob);
    std::vector<Command> expected = {
        Command(CommandType::SET, {"RADU", "10252"}),
        Command(CommandType::SET, {"RAZVAN", "vac"}),
        Command(CommandType::SELECT, {"RAZVAN"}),
        Command(CommandType::DEL, {"RADU"}),
        Command(CommandType::SHOW, {}),
        Command(CommandType::SHOW_LOCAL, {}),
        Command(CommandType::SHOW_WRITE, {})
    };

    CHECK(res.size() == expected.size());
    for (size_t i = 0; i < res.size(); i++) {
        CHECK(res[i].get_type() == expected[i].get_type());
        CHECK(res[i].get_args() == expected[i].get_args());
    }
}

TEST_CASE("Test transactions") {
    std::string blob =
        "MULTI\n"
        "SET RADU 10\n"
        "SET RAZVAN 30\n"
        "SELECT RAZVAN\n"
        "DEL RADU\n"
        "EXEC\n";

    auto res = Command::parse(blob);
    std::vector<Command> expected = {
        Command(CommandType::MULTI, {}),
        Command(CommandType::SET, {"RADU", "10"}),
        Command(CommandType::SET, {"RAZVAN", "30"}),
        Command(CommandType::SELECT, {"RAZVAN"}),
        Command(CommandType::DEL, {"RADU"}),
        Command(CommandType::EXEC, {}),
    };

    CHECK(res.size() == expected.size());
    for (size_t i = 0; i < res.size(); i++) {
        CHECK(res[i].get_type() == expected[i].get_type());
        CHECK(res[i].get_args() == expected[i].get_args());
    }
}

TEST_CASE("Test wrong commands") {
    std::string blob =
        "ST RADU 10\n"
        "SET RAZVAN 30\n"
        "SLECT RAZVAN\n"
        "DET RADU\n"
        "SET AURICA 420\n"
        "DET\n"
        "SLECT\n"
        "ST RADU\n"
        "ST\n";

    auto res = Command::parse(blob);
    std::vector<Command> expected = {
        Command(CommandType::SET, {"RAZVAN", "30"}),
        Command(CommandType::SET, {"AURICA", "420"}),
    };

    CHECK(res.size() == expected.size());
    for (size_t i = 0; i < res.size(); i++) {
        CHECK(res[i].get_type() == expected[i].get_type());
        CHECK(res[i].get_args() == expected[i].get_args());
    }
}
