#include <iostream>
#include <print>
#include <string>
#include <vector>

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
    bool buffering = false;
    std::vector<std::string> command_buffer;

    while (running) {
        if (failed_commands == ALLOWED_FAILS) {
            std::println(std::cerr,
                         "[ERROR]: Connection to the server is unreliable!");
            std::println("Terminating session...");
            return 1;
        }

        std::string command;
        std::print("{}", buffering ? "(MULTI)> " : "> ");

        std::getline(std::cin, command);

        if (command == "MULTI" && !buffering) {
            buffering = true;
            command_buffer.clear();
            command_buffer.push_back(command);
            std::println("Entering MULTI mode. Type EXEC to send commands.");
        } else if (command == "EXEC") {
            if (!buffering) {
                std::println("EXEC can only be called only in MULTI mode");
                continue;
            }

            command_buffer.push_back(command);
            std::string joined;
            for (const auto &cmd : command_buffer) {
                joined += cmd + '\n';
            }

            auto res = conn.send_msg(joined);
            if (res != std::nullopt) {
                failed_commands++;
                std::println(std::cerr, "{}", res.value().message());
            } else {
                std::println("All MULTI commands sent!");
                failed_commands = 0;
            }

            buffering = false;
            command_buffer.clear();
        } else {
            if (!buffering) {
                auto res = conn.send_msg(command);
                if (res != std::nullopt) {
                    failed_commands++;
                    std::println(std::cerr, "{}", res.value().message());
                } else {
                    std::println("Command sent!");
                    failed_commands = 0;
                }
            } else {
                command_buffer.push_back(command);
            }
        }
    }

    return 0;
}
