#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_ROOT"

echo "Cleaning build and install directories..."
rm -rf -- build install

echo "Configuring CMake..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

echo "Building project..."
cmake --build build --config Release

echo "Installing project..."
cmake --install build --prefix "$PROJECT_ROOT/install" --config Release

echo
echo "Build completed successfully!"
