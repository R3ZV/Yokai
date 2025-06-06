#include <iostream>
#include <print>

#include "../common/include/connection.h"

int main() {
    constexpr int32_t PORT = 8080;
    Connection conn = Connection(PORT);
    std::optional<std::error_code> err = conn.init_client("127.0.0.1");
    if (err != std::nullopt) {
        std::println(std::cerr, "{}", err.value().message());
        return 1;
    }

    int failed_commands = 0;
    constexpr int ALLOWED_FAILS = 5;

    bool running = true;
    while (running) {
        if (failed_commands == ALLOWED_FAILS) {
            std::println(std::cerr,
                         "[ERROR]: Connection to the server is unreliable!");
            std::println("Terminating session...");
            running = false;
            return 1;
        }
        std::string command;
        std::print("> ");
        std::getline(std::cin, command);
        std::println(std::cerr, "[DBG]: Command send = '{}'", command);

        auto res = conn.send_msg(command);
        if (res != std::nullopt) {
            failed_commands++;
            std::println(std::cerr, "{}", res.value().message());
        } else {
            std::println("Command sent!");
            failed_commands = 0;
        }
    }
    return 0;
}
