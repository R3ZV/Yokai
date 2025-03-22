#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[ERROR]: Unable to create socket!" << std::endl;
        close(server_fd);
        return 1;
    }

    constexpr int PORT = 8080;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
        std::cerr << "[ERROR]: Invalid address/ Address not supported!" << std::endl;
        close(server_fd);
        return 1;
    }

    if (connect(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "[ERROR]: Connection failed, server doesn't exist!" << std::endl;
        close(server_fd);
        return 1;
    }

    bool running = true;
    int failed_commands = 0;
    constexpr int ALLOWED_FAILS = 5;
    while (running) {
        if (failed_commands == ALLOWED_FAILS) {
            std::cerr << "[ERROR]: Connection to the server is unreliable!" << std::endl;
            std::cerr << "Terminating session..." << std::endl;
            running = false;
            close(server_fd);
            return 1;
        }

        std::string command;
        std::print("> ");
        std::getline(std::cin, command);
        std::cerr << "[DBG]: Command send = '"  << command << "'" << std::endl;

        if (send(server_fd, &command, command.size(), 0) == -1) {
            failed_commands++;
            std::cerr << "[ERROR]: Sending data to the server failed!" << std::endl;
            return 1;
        } else {
            std::cerr << "[INFO]: Message sent!" << std::endl;
            failed_commands = 0;
        }
    }

    close(server_fd);
    return 0;
}
