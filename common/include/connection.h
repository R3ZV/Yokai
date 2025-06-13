#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <expected>
#include <iostream>

class Connection {
    int32_t socket_fd;
    struct sockaddr_in address;
    const int32_t PORT;

   public:
    Connection(int32_t port);
    ~Connection();

    auto init_server(const int32_t clients) -> std::optional<std::error_code>;
    auto init_client(const char *addr) -> std::optional<std::error_code>;

    // Returns a file descriptor to the client.
    // NOTE: It is the job of the caller to close the connection
    auto accept_conn() -> std::expected<int32_t, std::error_code>;
    auto send_msg(std::string msg) -> std::optional<std::error_code>;
    auto get_socket_fd() const -> int32_t;
    static auto reply(int32_t client_fd, std::string msg)
        -> std::optional<std::error_code>;
};
