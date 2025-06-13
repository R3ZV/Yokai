#include <string.h>
#include <sys/wait.h>

#include <fstream>
#include <print>
#include <thread>

#include "../common/include/connection.h"
#include "include/list_database.h"
#include "include/transaction.h"

// Interval in seconds for saving data
constexpr int SAVE_INTERVAL = 10;

std::atomic<bool> running = true;

auto handle_client(int client, ListDatabase* db) -> void {
    constexpr int BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];
    Transaction* user_transaction = new Transaction(db);

    while (true) {
        memset(buff, 0, BUFF_SIZE);
        ssize_t bytes_read = read(client, buff, BUFF_SIZE);
        std::println(std::cerr, "\n[DBG]: Number of bytes read = {}",
                     bytes_read);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::println("[INFO]: Client '{}' disconnected!", client);
            } else {
                std::println(
                    std::cerr,
                    "[ERROR]: Read failed or connection closed by client");
            }
            break;
        }
        std::println("Message from client: {}", buff);

        auto res = user_transaction->handle_command(buff);
        Connection::reply(client, res);
    }

    close(client);
}

auto try_backup(ListDatabase* db) -> std::expected<void, std::string> {
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
        std::println(out, "{} {}", key, latest.value()->encode());
    }

    out.close();

    std::string final_file = "saves/dump.txt";
    if (std::rename(temp_file.c_str(), final_file.c_str()) != 0) {
        std::perror("rename");
        return std::unexpected("Failed to rename temp file to final file");
    }

    return {};
}

auto save_loop(ListDatabase* db) -> void {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(SAVE_INTERVAL));
        std::println(std::cerr, "[DBG] Trying to save");

        pid_t pid = fork();
        if (pid == 0) {
            auto res = try_backup(db);
            if (!res.has_value()) {
                std::println(std::cerr, "[ERR] Save failed: {}", res.error());
                return;
            }
            std::println(std::cerr, "[DBG] Saved data");
            exit(0);
        } else if (pid < 0) {
            std::println(std::cerr, "Failed to fork process for saving.");
        }

        // Clean up children
        int status;
        pid_t finished_pid;
        while ((finished_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                std::println(std::cerr, "[DBG] Child {} exited with code {}",
                             finished_pid, exit_code);
            } else if (WIFSIGNALED(status)) {
                std::println(std::cerr,
                             "[DBG] Child {} was terminated by signal {}",
                             finished_pid, WTERMSIG(status));
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
        std::println(std::cerr, "{}", err.value().message());
        return 1;
    }

    // Load data
    auto res = db->load_from_file();
    if (!res.has_value()) {
        std::println(std::cerr, "{}", res.error());
        return -1;
    }

    // Create background thread for saving data
    std::thread save_thread(save_loop, db);

    bool running = true;
    std::println("Listenting for connections on port :{}", PORT);
    while (running) {
        auto client_fd = conn.accept_conn();
        if (!client_fd.has_value()) {
            std::println(std::cerr, "{}", client_fd.error().message());
        }

        std::thread client_thread(handle_client, client_fd.value(), db);
        client_thread.detach();
    }
    save_thread.join();
    return 0;
}
