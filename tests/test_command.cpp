#include "../daemon/include/command.h"
#include "../lib/doctest.h"

TEST_CASE("Test non-transactions") {
    std::vector<std::string> blobs = {
        "SET RADU 10252\n", "SET RAZVAN vac\n", "SELECT RAZVAN\n", "DEL RADU\n",
        "SHOW\n",           "SHOW LOCAL\n",     "SHOW WRITE\n"};

    std::vector<Command> expected = {
        Command(CommandType::SET, {"RADU", "10252"}),
        Command(CommandType::SET, {"RAZVAN", "vac"}),
        Command(CommandType::SELECT, {"RAZVAN"}),
        Command(CommandType::DEL, {"RADU"}),
        Command(CommandType::SHOW, {}),
        Command(CommandType::SHOW_LOCAL, {}),
        Command(CommandType::SHOW_WRITE, {})};

    for (size_t i = 0; i < expected.size(); i++) {
        auto res = Command::parse(blobs[i])[0];
        CHECK(res.get_type() == expected[i].get_type());
        CHECK(res.get_args() == expected[i].get_args());
    }
}

TEST_CASE("Test transactions") {
    std::vector<std::string> blobs = {"MULTI\n",         "SET RADU 10\n",
                                      "SET RAZVAN 30\n", "SELECT RAZVAN\n",
                                      "DEL RADU\n",      "EXEC\n"};

    std::vector<Command> expected = {
        Command(CommandType::MULTI, {}),
        Command(CommandType::SET, {"RADU", "10"}),
        Command(CommandType::SET, {"RAZVAN", "30"}),
        Command(CommandType::SELECT, {"RAZVAN"}),
        Command(CommandType::DEL, {"RADU"}),
        Command(CommandType::EXEC, {}),
    };

    for (size_t i = 0; i < expected.size(); i++) {
        auto res = Command::parse(blobs[i])[0];
        CHECK(res.get_type() == expected[i].get_type());
        CHECK(res.get_args() == expected[i].get_args());
    }
}

TEST_CASE("Test wrong commands") {
    std::vector<std::string> blobs = {
        "ST RADU 10\n", "SET RAZVAN 30\n",  "SLECT RAZVAN\n",
        "DET RADU\n",   "SET AURICA 420\n", "DET\n",
        "SLECT\n",      "ST RADU\n",        "ST\n",
    };

    std::vector<Command> expected = {
        Command(CommandType::INVALID, {}),
        Command(CommandType::SET, {"RAZVAN", "30"}),
        Command(CommandType::INVALID, {}),
        Command(CommandType::INVALID, {}),
        Command(CommandType::SET, {"AURICA", "420"}),
        Command(CommandType::INVALID, {}),
        Command(CommandType::INVALID, {}),
        Command(CommandType::INVALID, {}),
        Command(CommandType::INVALID, {}),
    };

    for (size_t i = 0; i < expected.size(); i++) {
        auto res = Command::parse(blobs[i])[0];

        CHECK(res.get_type() == expected[i].get_type());
        CHECK(res.get_args() == expected[i].get_args());
    }
}
