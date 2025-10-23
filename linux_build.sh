
# Absolute path to the script
SCRIPT_PATH="$(realpath "$0")"

# Directory containing the script
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"


gcc $SCRIPT_DIR/src/main.c -o $SCRIPT_DIR/borderless.so -lm -lSDL2

