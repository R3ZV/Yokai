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
    command_buffer = []

    style = Style.from_dict({
        COLOR_EXACT: 'bold #ffffff',
        COLOR_FUZZY: 'italic #000000',
    })

    while running:
        try:

            if failed_commands == ALLOWED_FAILS:
                print("[ERROR]: Connection to the server is unreliable!")
                print("Terminating session...")
                return 1

            try:
                command = prompt("(MULTI)> " if buffering else "> ", completer=RedisCompleter(), style=style)
            except EOFError:
                return 1

            if command == "exit":
                print("Bye.")
                return 0

            if command == "MULTI" and not buffering:
                buffering = True
                command_buffer.clear()
                command_buffer.append(command)
                print("Entering MULTI mode. Type EXEC to send commands.")
            elif command == "EXEC":
                if not buffering:
                    print("EXEC can only be called only in MULTI mode")
                    continue

                command_buffer.append(command)
                joined = '\n'.join(command_buffer) + '\n'

                res = conn.send_msg(joined)
                if res is not None:
                    failed_commands += 1
                    print(f"[ERROR]: {res}")
                else:
                    print("All MULTI commands sent!")
                    failed_commands = 0

                buffering = False
                command_buffer.clear()
            else:
                if not buffering:
                    res = conn.send_msg(command)
                    if res is not None:
                        failed_commands += 1
                        print(f"[ERROR]: {res}")
                    else:
                        print("Command sent!")
                        failed_commands = 0

                        # TODO: Wait for the returned message
                        # and print it to the user
                else:
                    command_buffer.append(command)



        # Handle Ctrl+C or Ctrl+D to exit 
        except (KeyboardInterrupt, EOFError):
            print("\nBye.")
            return 0

    return 0

if __name__ == "__main__":
    raise SystemExit(main())
