from connection import Connection
from completor import RedisCompleter, COLOR_EXACT, COLOR_FUZZY
from prompt_toolkit.styles import Style
from prompt_toolkit import prompt

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
                        # response already has \n
                        print(response, end="")
                command_buffer.clear()
                buffering = False

        # Handle Ctrl+C or Ctrl+D to exit 
        except (KeyboardInterrupt, EOFError):
            print("May the force be with you!")
            return 0

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
