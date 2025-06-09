#include <iostream>
#include <print>
#include <chrono>
#include <thread>

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
    constexpr auto TIMEOUT_DURATION = std::chrono::minutes(10);

    bool running = true;
    while (running) {
        if (failed_commands == ALLOWED_FAILS) {
            std::println(std::cerr,
                         "[ERROR]: Connection to the server is unreliable!");
            std::println("Terminating session...");
            return 1;
        }

        std::string command;
        std::print("> ");
        auto start = std::chrono::steady_clock::now();

        while (!std::cin.rdbuf()->in_avail()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto now = std::chrono::steady_clock::now();
            if (now - start > TIMEOUT_DURATION) {
                std::println("Connection timed out due to inactivity");
                std::println("Terminating session...");
                return 0;
            }
        }

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
