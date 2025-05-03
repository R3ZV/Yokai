#pragma once

#include <string>
#include <vector>

/// Following commands are supported:
/// SET [key] [val]
/// DEL [key]
/// SELECT [key]
/// SHOW  (shows global dict)
/// SHOW LOCAL  (shows thread local dict/read buffer)
/// SHOW WRITE  (shows write buffer)
/// MULTI  (begin multiple command transaction)
/// EXEC  (execute multiple command transaction)
enum CommandType {
    SET,
    DEL,
    SELECT,
    SHOW,
    SHOW_LOCAL,
    SHOW_WRITE,
    MULTI,
    EXEC,
};

class Command {
    const CommandType type;
    const std::vector<std::string> args;

   public:
    Command(CommandType type, std::vector<std::string> args)
        : type(type), args(args) {}

    /// `parse` assumes that the `blob` given is a string which may
    /// include multiple lines.
    static auto parse(const std::string& blob) -> std::vector<Command>;

    auto get_type() -> CommandType;
    auto get_args() -> std::vector<std::string>;
};
