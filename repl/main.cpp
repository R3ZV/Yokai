#include <iostream>

#include "../common/include/connection.h"

int main() {
    constexpr int32_t PORT = 8080;
    Connection conn = Connection(PORT);
    std::optional<std::error_code> err = conn.init_client("127.0.0.1");
    if (err != std::nullopt) {
        std::cerr << err.value().message() << std::endl;
        return 1;
    }

    int failed_commands = 0;
    constexpr int ALLOWED_FAILS = 5;

    bool running = true;
    while (running) {
        if (failed_commands == ALLOWED_FAILS) {
            std::cerr << "[ERROR]: Connection to the server is unreliable!"
                      << std::endl;
            std::cerr << "Terminating session..." << std::endl;
            running = false;
            return 1;
        }

        std::string command;
        std::print("> ");
        std::getline(std::cin, command);
        std::cerr << "[DBG]: Command send = '" << command << "'" << std::endl;

        auto res = conn.send_msg(command);
        if (res != std::nullopt) {
            failed_commands++;
            std::cerr << res.value().message() << std::endl;
        } else {
            std::cerr << "[INFO]: Message sent!" << std::endl;
            failed_commands = 0;
        }
    }
    return 0;
}
