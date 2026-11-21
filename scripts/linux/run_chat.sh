#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
INSTALL_DIR="$PROJECT_ROOT/install/linux"
EXE_PATH="$INSTALL_DIR/bin/comms_manager"
CONFIG_CREATOR="$INSTALL_DIR/config"
CONFIG_JOINER="$INSTALL_DIR/config_joiner"

for path in "$EXE_PATH" "$CONFIG_CREATOR/config.yaml" "$CONFIG_JOINER/config.yaml"; do
    if [[ ! -f "$path" ]]; then
        echo "Error: required file not found: $path" >&2
        exit 1
    fi
done

if command -v x-terminal-emulator >/dev/null 2>&1; then
    launch_terminal() { x-terminal-emulator -e "$@" & }
elif command -v gnome-terminal >/dev/null 2>&1; then
    launch_terminal() { gnome-terminal -- "$@" & }
elif command -v konsole >/dev/null 2>&1; then
    launch_terminal() { konsole -e "$@" & }
elif command -v xfce4-terminal >/dev/null 2>&1; then
    launch_terminal() { xfce4-terminal --execute "$@" & }
elif command -v xterm >/dev/null 2>&1; then
    launch_terminal() { xterm -e "$@" & }
else
    echo "Error: no supported terminal emulator found (x-terminal-emulator, gnome-terminal, konsole, xfce4-terminal, or xterm)." >&2
    exit 1
fi

echo "Starting Chat Creator (Server)..."
launch_terminal "$EXE_PATH" --config_folder "$CONFIG_CREATOR"

sleep 2

echo "Starting Chat Joiner (Client)..."
launch_terminal "$EXE_PATH" --config_folder "$CONFIG_JOINER"

echo "Both instances are running in separate terminal windows."
