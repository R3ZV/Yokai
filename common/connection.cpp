#include "include/connection.h"

#include <cstring>

Connection::Connection(int32_t port) : PORT(port) {}

Connection::~Connection() { close(socket_fd); }

auto Connection::init_server(const int clients)
    -> std::optional<std::error_code> {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    constexpr int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    if (listen(socket_fd, clients) == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    return std::nullopt;
}

auto Connection::init_client(const char *addr)
    -> std::optional<std::error_code> {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, addr, &address.sin_addr) == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) ==
        -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }

    return std::nullopt;
}

auto Connection::accept_conn() -> std::expected<int32_t, std::error_code> {
    const int32_t addrlen = sizeof(address);
    int32_t client =
        accept(socket_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client == -1) {
        return std::unexpected(std::error_code(errno, std::generic_category()));
    }
    return client;
}

auto Connection::send_msg(std::string msg) -> std::optional<std::error_code> {
    constexpr int BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];
    strcpy(buff, msg.c_str());

    if (send(socket_fd, buff, msg.size() + 1, 0) == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }
    return std::nullopt;
}

auto Connection::get_socket_fd() const -> int32_t { return socket_fd; }

auto Connection::reply(int32_t client_fd, std::string msg)
    -> std::optional<std::error_code> {
    constexpr int BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];
    strcpy(buff, msg.c_str());

    if (send(client_fd, buff, msg.size() + 1, 0) == -1) {
        return std::make_optional(
            std::error_code(errno, std::generic_category()));
    }
    return std::nullopt;
}
