#include <cstring>
#include <iostream>
#include <thread>
#include <print>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

void handle_client(int client) {
    constexpr int BUFF_SIZE = 1024;
    char buff[BUFF_SIZE] = {0};
    while (true) {
        memset(&buff, 0, BUFF_SIZE);
        ssize_t bytes_read = read(client, &buff, BUFF_SIZE - 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::println("[INFO]: Client '{}' disconnected!", client);
            } else {
                std::cerr << "[ERROR]: Read failed or connection closed by client" << std::endl;
            }
            break;
        }
        std::println("Message from client: {}", buff);
    }

    close(client);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "[ERROR]: Unable to create socket" << std::endl;
        close(server_fd);
        return 1;
    }

    constexpr int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "[ERROR]: Setting socket opt failed" << std::endl;
        close(server_fd);
        return 1;
    }

    constexpr int PORT = 8080;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "[ERROR]: Binding failed" << std::endl;
        close(server_fd);
        return 1;
    }

    constexpr int NUM_CLIENTS = 3;
    if (listen(server_fd, NUM_CLIENTS) < 0) {
        std::cerr << "[ERROR]: Servern couldn't listen" << std::endl;
        close(server_fd);
        return 1;
    }

    constexpr int addrlen = sizeof(address);
    bool running = true;
    std::println("Listenting for connections on port :{}", PORT);
    while (running) {
        int client = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client < 0) {
            std::cerr << "[ERROR]: Couldn't accept client" << std::endl;
            continue;
            return 1;
        }

        std::thread client_thread(handle_client, client);
        client_thread.detach();
    }
    close(server_fd);
    return 0;
}
