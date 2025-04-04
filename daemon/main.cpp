#include <print>
#include <thread>
#include<atomic>
#include<chrono>
#include<poll.h>
#include "../common/include/connection.h"

void handle_client(int client) {
    constexpr int BUFF_SIZE = 1024;
    constexpr int TIMEOUT_SECONDS=10;
    char buff[BUFF_SIZE];
    pollfd pfd; //we'll need this to use poll(), see linux manual
    pfd.fd=client;
    pfd.events=POLLIN;
    while (true) {
        memset(buff, 0, BUFF_SIZE);
        //here's the magic: poll() allows us to check if a fd is ready for writing in it
        int ready=poll(&pfd, 1, TIMEOUT_SECONDS*1000);
        if(ready==0){
            std::println("[WARNING]: Your time has run out, you'll be disconnected");
            break;
        }
        ssize_t bytes_read = read(client, buff, BUFF_SIZE);
	std::println("[DBG]: Number of bytes read = {}", bytes_read);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::println("[INFO]: Client '{}' disconnected!", client);
            } else {
                std::cerr
                    << "[ERROR]: Read failed or connection closed by client"
                    << std::endl;
            }
            break;
        }
        std::println("Message from client: {}", buff);
    }
    
    close(client);
}

int main() {
    constexpr int32_t PORT = 8080;
    constexpr int32_t NUM_CLIENTS = 3;

    Connection conn(PORT);
    std::optional<std::error_code> err = conn.init_server(NUM_CLIENTS);
    if (err != std::nullopt) {
        std::cerr << err.value().message() << std::endl;
        return 1;
    }

    bool running = true;
    std::println("Listenting for connections on port :{}", PORT);
    while (running) {
        auto client_fd = conn.accept_conn();
        if (!client_fd.has_value()) {
            std::cerr << client_fd.error().message() << std::endl;
        }

        std::thread client_thread(handle_client, client_fd.value());
        client_thread.detach();
    }
    return 0;
}
