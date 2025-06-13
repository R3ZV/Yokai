# For fuzzy matching of commands 
from fuzzywuzzy import process

# For implementing auto-completion logic
from prompt_toolkit.completion import Completer, Completion

# Color for exact command match
COLOR_EXACT = "ansigreen"

# Color for fuzzy/approximate match
COLOR_FUZZY = "ansiyellow"

# Define supported commands with their expected syntax
COMMANDS = {
    "SET": "SET key val",
    "DEL": "DEL key",
    "SELECT": "SELECT key",
    "SHOW": "SHOW",
    "SHOW LOCAL": "SHOW LOCAL",
    "SHOW WRITE": "SHOW WRITE",
    "MULTI": "MULTI",
    "EXEC": "EXEC",
    "ROLLBACK": "ROLLBACK"
}

# Custom completer class for command-line auto-suggestions
class RedisCompleter(Completer):
    def get_completions(self, document, complete_event):
        # Extract and split user input
        text = document.text.strip()
        tokens = text.split()

        # If no input, suggest all commands
        if not tokens:
            for cmd in sorted(COMMANDS):
                yield Completion(COMMANDS[cmd], style=COLOR_EXACT, start_position=0)
            return

        # Extract command and arguments
        cmd = tokens[0].upper()

        # Handle autocompletion when user has typed only the command
        if len(tokens) == 1:
            # Exact matches (start with the typed text)
            exact_matches = [c for c in COMMANDS if c.startswith(cmd)]
            # Fuzzy matches (similar commands based on fuzzy ratio)
            fuzzy_matches = [m[0] for m in process.extract(cmd, COMMANDS.keys(), limit=5) if m[1] > 70]

            # Yield exact match suggestions
            for c in sorted(set(exact_matches)):
                yield Completion(COMMANDS[c], style=COLOR_EXACT, start_position=-len(tokens[0]))

            # Yield fuzzy match suggestions (if not already exact matches)
            for f in sorted(set(fuzzy_matches) - set(exact_matches)):
                yield Completion(COMMANDS[f], style=COLOR_FUZZY, start_position=-len(tokens[0]))

        # If command is complete and arguments are being typed, suggest the full syntax
        else:
            if cmd in COMMANDS:
                args_hint = COMMANDS[cmd]
                yield Completion(args_hint, style=COLOR_EXACT, start_position=-len(' '.join(tokens)))
