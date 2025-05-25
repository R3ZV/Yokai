#include <string.h>
#include <wait.h>

#include <fstream>
#include <print>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../common/include/connection.h"
#include "include/database.h"
#include "include/list_database.h"
#include "include/transaction.h"

// Interval in seconds for saving data
constexpr int SAVE_INTERVAL = 10;

std::atomic<bool> running{true};

void handle_client(int client, ListDatabase* db) {
    constexpr int BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];
    Transaction* user_transaction = new Transaction(db);

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

        user_transaction->handle_command(buff);
    }

    close(client);
}

std::expected<void, std::string> save_to_debug_file(ListDatabase* db) {
    std::string temp_file = "saves/dump.txt.temp";

    std::ofstream out(temp_file);
    if (!out) {
        return std::unexpected("Could not open file for writing: " + temp_file);
    }

    for (const auto& [key, value] : db->get_data()) {
        auto latest = db->select_latest(key, Object::get_current_time());
        if (!latest.has_value() || !latest.value()) {
            return std::unexpected("Failed to get latest object for key: " +
                                   key);
        }
        out << key << '\t' << latest.value()->encode() << "\n";
    }

    out.close();

    std::string final_file = "saves/dump.txt";
    if (std::rename(temp_file.c_str(), final_file.c_str()) != 0) {
        std::perror("rename");
        return std::unexpected("Failed to rename temp file to final file");
    }

    return {};
}

void save_loop(ListDatabase* db) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(SAVE_INTERVAL));
        std::println("[DBG] Trying to save");

        pid_t pid = fork();
        if (pid == 0) {
            auto res = save_to_debug_file(db);
            if (!res.has_value()) {
                std::cerr << "[ERR] Save failed: " << res.error() << "\n";
                return;
            }
            std::cout << "[DBG] Saved data" << std::endl;
            exit(0);
        } else if (pid < 0) {
            std::cerr << "Failed to fork process for saving." << std::endl;
        }

        // Clean up children
        int status;
        pid_t finished_pid;
        while ((finished_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                std::cout << "[DBG] Child " << finished_pid
                          << " exited with code " << exit_code << "\n";
            } else if (WIFSIGNALED(status)) {
                std::cerr << "[DBG] Child " << finished_pid
                          << " was terminated by signal " << WTERMSIG(status)
                          << "\n";
            }
        }
    }
}

int main() {
    ListDatabase* db = new ListDatabase();

    constexpr int32_t PORT = 8080;
    constexpr int32_t NUM_CLIENTS = 3;

    Connection conn(PORT);
    std::optional<std::error_code> err = conn.init_server(NUM_CLIENTS);
    if (err != std::nullopt) {
        std::cerr << err.value().message() << std::endl;
        return 1;
    }

    // Load data
    auto res = db->load_from_file();
    if (!res.has_value()) {
        std::cerr << res.error();
        return -1;
    }

    // Create background thread for saving data
    std::thread save_thread(save_loop, db);

    bool running = true;
    std::println("Listenting for connections on port :{}", PORT);
    while (running) {
        auto client_fd = conn.accept_conn();
        if (!client_fd.has_value()) {
            std::cerr << client_fd.error().message() << std::endl;
        }

        std::thread client_thread(handle_client, client_fd.value(), db);
        client_thread.detach();
    }
    save_thread.join();
    return 0;
}
