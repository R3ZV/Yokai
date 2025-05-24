#include <string.h>

#include <print>
#include <thread>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <wait.h>

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

std::string generate_timestamped_filename() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << "saves/debug-";
    oss << std::put_time(std::localtime(&time_now), "%Y%m%d-%H%M%S");
    oss << ".txt";

    return oss.str();
}

std::string save_to_debug_file(const std::map<std::string, std::deque<std::shared_ptr<Object>>> & db) {
    std::string filename = generate_timestamped_filename();

    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    for (const auto& [key, value] : db) {
        out << key << '\t' << value.size() << '\n';
    }

    out.close();
    return filename;
}

void save_loop(ListDatabase* db) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(SAVE_INTERVAL));
        std::println("[DBG] Trying to save");

        pid_t pid = fork();
        if (pid == 0) {
            try {
                std::string filename = save_to_debug_file(db->get_data());
                std::cout << "[DBG] Saved data to " << filename << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[DBG] Save failed: " << e.what() << std::endl;
            }
            _exit(0);
        } else if (pid < 0) {
            std::cerr << "Failed to fork process for saving." << std::endl;
        }

        // Clean up children
        int status;
        pid_t finished_pid;
        while ((finished_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                std::cout << "[DBG] Child " << finished_pid << " exited with code " << exit_code << "\n";
            } else if (WIFSIGNALED(status)) {
                std::cerr << "[DBG] Child " << finished_pid << " was terminated by signal " << WTERMSIG(status) << "\n";
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
