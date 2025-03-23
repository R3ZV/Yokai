#pragma once

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>
#include <cerrno>
#include <expected>

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
};
