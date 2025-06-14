from connection import Connection
from completor import RedisCompleter, COLOR_EXACT, COLOR_FUZZY
from prompt_toolkit.styles import Style
from prompt_toolkit import prompt
from validator import is_valid_redis_command
import difflib

VALID_COMMANDS = [
    "SADD", "SET", "DEL", "SELECT", "SHOW", "SHOW LOCAL", "SHOW WRITE",
    "MULTI", "EXEC", "ROLLBACK"
]

# Suggests the closest command, only if the misspelled command and the correct one match at least 20%
def suggest_command(cmd: str) -> str | None:
    parts = cmd.strip().split()
    if not parts:
        return None

    user_cmd = parts[0].upper()
    best = difflib.get_close_matches(user_cmd, VALID_COMMANDS, n=1, cutoff=0.2)
    return best[0] if best else None

def main() -> int:
    PORT = 8080
    conn = Connection(PORT)
    err = conn.init_client("127.0.0.1")

    if err is not None:
        print(f"[ERROR]: {err}")
        return 1

    failed_commands = 0
    ALLOWED_FAILS = 5

    running = True
    buffering = False
    exec = False
    command_buffer = []

    style = Style.from_dict({
        COLOR_EXACT: 'bold #ffffff',
        COLOR_FUZZY: 'italic #000000',
    })

    while running:
        if failed_commands == ALLOWED_FAILS:
            print("[ERROR]: Connection to the server is unreliable!")
            print("Terminating session...")
            return 1

        try:
            command = prompt("(MULTI)> " if buffering else "> ", completer=RedisCompleter(), style=style)
            if command == "exit":
                print("May the force be with you!")
                return 0

            is_valid, err_msg = is_valid_redis_command(command)
            if not is_valid:
                if err_msg == "UNKNOWN":
                    suggestion = suggest_command(command)
                    if suggestion:
                        print(f"Wrong command, did you mean {suggestion}?")
                else:
                    print(f"{err_msg}")

                continue

            if command == "MULTI":
                if not buffering:
                    buffering = True
                    print("Entering MULTI mode. Type EXEC to send commands.")
            elif command == "EXEC":
                if not buffering:
                    print("EXEC can only be called only in MULTI mode")
                    continue
                exec = True
            else:
                command_buffer.append(command)
                if not buffering:
                    exec = True

            if exec:
                exec = False
                joined = '\n'.join(command_buffer)

                res = conn.send_msg(joined)
                if res is not None:
                    failed_commands += 1
                    print(f"[ERROR]: {res}")
                else:
                    failed_commands = 0
                    response = conn.receive_msg()
                    if response is not None:
                        print(response, end="")
                command_buffer.clear()
                buffering = False

        except (KeyboardInterrupt, EOFError):
            print("May the force be with you!")
            return 0

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
