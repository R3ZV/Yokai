#pragma once

#include <iostream>
#include <print>
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
    INVALID,
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

    auto get_type() const -> CommandType;
    auto get_args() const -> std::vector<std::string>;
};

namespace std {
template <>
struct formatter<Command> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const Command& cmd, format_context& ctx) const {
        std::string type_str;
        switch (cmd.get_type()) {
            case SET:
                type_str = "SET";
                break;
            case DEL:
                type_str = "DEL";
                break;
            case SELECT:
                type_str = "SELECT";
                break;
            case SHOW:
                type_str = "SHOW";
                break;
            case SHOW_LOCAL:
                type_str = "SHOW LOCAL";
                break;
            case SHOW_WRITE:
                type_str = "SHOW WRITE";
                break;
            case MULTI:
                type_str = "MULTI";
                break;
            case EXEC:
                type_str = "EXEC";
                break;
            case INVALID:
                type_str = "INVALID";
                break;
            default:
                type_str = "UNKNOWN";
                break;
        }

        auto out = format_to(ctx.out(), "{}", type_str);
        for (const auto& arg : cmd.get_args()) {
            out = format_to(out, " {}", arg);
        }
        return out;
    }
};
}  // namespace std
