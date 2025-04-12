#pragma GCC diagnostic ignored "-Wunused-variable"

#include <print>
#include <thread>
#include <vector>

#include "../common/include/connection.h"
#include "../common/include/database.h"
#include <string.h>

// create global database
Database* global_dict = new Database();

void handle_client(int client) {
    constexpr int BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];
    while (true) {
        memset(buff, 0, BUFF_SIZE);
        ssize_t bytes_read = read(client, buff, BUFF_SIZE);
	std::println("\n[DBG]: Number of bytes read = {}", bytes_read);
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

        // check if the message is SET smth or DELETE smth
        // also SHOW to show entire database
        std::vector<std::string> tokens;
        char* token = strtok(buff, " ");

        while (token != nullptr) {
            // Add token to vector
            tokens.push_back(token);
    
            // Get the next token
            token = strtok(nullptr, " ");
        }
        
        std::println("[BDG]: Tokens: ");
        for (const auto& t : tokens) {
            std::println("{}", t);
        }
        
        // SET
        if (tokens.size() == 3 && tokens[0] == "SET") {            
            std::println("[DBG]: Setting key {} to value {}", tokens[1], tokens[2]);            
        }
        // DELETE
        else if (tokens.size() == 2 && tokens[0] == "DEL") {
            std::println("[DBG]: Deleting key {}", tokens[1]);
        }
        // SHOW DATA
        else if (tokens.size() == 1 && tokens[0] == "SHOW") {
            global_dict->show_objects();
        }
        else {
            std::println("Not a recognised command");
        }
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
