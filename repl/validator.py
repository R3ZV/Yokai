def is_valid_redis_command(command: str) -> tuple[bool, str | None]:
    """
    Validates a command string based on our custom Redis-like grammar.
    Returns (True, None) if valid, else (False, error message)
    """

    # value of the key = number of expected arguments
    COMMANDS = {
        "SADD": 2,
        "SET": 2,
        "DEL": 1,
        "SELECT": 1,
        "SHOW": 0,
        "SHOW LOCAL": 0,
        "SHOW WRITE": 0,
        "MULTI": 0,
        "EXEC": 0,
        "ROLLBACK": 0,
    }

    tokens = command.strip().split()
    if not tokens:
        return False, "Empty command"

    # Try to match two-word commands first
    cmd = tokens[0].upper()
    if len(tokens) >= 2:
        cmd2 = f"{tokens[0].upper()} {tokens[1].upper()}"
        if cmd2 in COMMANDS:
            expected_args = COMMANDS[cmd2]
            if len(tokens) - 2 != expected_args:
                return False, f"{cmd2} expects {expected_args} argument(s)"
            return True, None

    if cmd in COMMANDS:
        expected_args = COMMANDS[cmd]
        if len(tokens) - 1 != expected_args:
            return False, f"{cmd} expects {expected_args} argument(s)"
        return True, None

    return False, "UNKNOWN"
