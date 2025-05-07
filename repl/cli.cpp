#include <iostream>
#include <string>
#include <sstream>
#include "../common/include/connection.h"  

void run_cli(Connection& conn) {
    std::string input, command, key, value;

    std::cout << "Yokai CLI started. Type 'EXIT' to quit.\n";

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input); 

        if (input == "EXIT") break; 

        std::istringstream iss(input);
        iss >> command >> key;

        if (command == "SET") {
            iss >> value;  // Extract value for SET
            if (key.empty() || value.empty()) {
                std::cerr << "Usage: SET <key> <value>\n";
                continue;
            }
            conn.send_message("SET " + key + " " + value);
        } 
        else if (command == "GET" || command == "DEL") {
            if (key.empty()) {
                std::cerr << "Usage: " << command << " <key>\n";
                continue;
            }
            conn.send_message(command + " " + key);
        } 
        else {
            std::cerr << "Unknown command: " << command << "\n";
            continue;
        }

        std::string response = conn.receive_message();
        std::cout << response << std::endl;
    }
}

int main() {
    try {
        Connection conn;
        conn.init_client("127.0.0.1", 8001);
        run_cli(conn);
    } catch (const std::exception& e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

