#include "include/command.h"

#include <sstream>
#include <string>

auto Command::parse(const std::string& blob) -> std::vector<Command> {
    std::vector<Command> commands;

    std::istringstream lines(blob);
    std::string line;
    while (std::getline(lines, line)) {
        std::vector<std::string> tokens;
        size_t idx = 0;
        for (size_t next_idx = line.find(" ", idx);
             next_idx != std::string::npos; next_idx = line.find(" ", idx)) {
            tokens.emplace_back(line.substr(idx, next_idx - idx));
            idx = next_idx + 1;
        }
        tokens.emplace_back(line.substr(idx, line.length() - idx));

        for (auto it = tokens.begin(); it != tokens.end(); it++) {
            if (*it == "SET") {
                if (it + 2 >= tokens.end()) {
                    continue;
                }
                commands.emplace_back(
                    Command(CommandType::SET, {*(it + 1), *(it + 2)}));
            } else if (*it == "SADD") {
                if (it + 2 >= tokens.end()) {
                    continue;
                }
                commands.emplace_back(
                    Command(CommandType::SADD, {*(it + 1), *(it + 2)}));
            } else if (*it == "DEL") {
                if (it + 1 >= tokens.end()) {
                    continue;
                }
                commands.emplace_back(Command(CommandType::DEL, {*(it + 1)}));
            } else if (*it == "SELECT") {
                if (it + 1 >= tokens.end()) {
                    continue;
                }
                commands.emplace_back(
                    Command(CommandType::SELECT, {*(it + 1)}));
            } else if (*it == "SHOW") {
                if (it + 1 >= tokens.end()) {
                    commands.emplace_back(Command(CommandType::SHOW, {}));
                } else {
                    if (*(it + 1) == "LOCAL") {
                        commands.emplace_back(
                            Command(CommandType::SHOW_LOCAL, {}));
                    } else if (*(it + 1) == "WRITE") {
                        commands.emplace_back(
                            Command(CommandType::SHOW_WRITE, {}));
                    } else {
                        continue;
                    }
                }
            } else if (*it == "EXEC") {
                commands.emplace_back(Command(CommandType::EXEC, {}));
            } else if (*it == "MULTI") {
                commands.emplace_back(Command(CommandType::MULTI, {}));
            } else if (*it == "ROLLBACK") {
                commands.emplace_back(Command(CommandType::ROLLBACK, {}));
            } else {
                commands.emplace_back(Command(CommandType::INVALID, {}));
            }
        }
    }

    return commands;
}

auto Command::get_type() const -> CommandType { return this->type; }
auto Command::get_args() const -> std::vector<std::string> {
    return this->args;
}
