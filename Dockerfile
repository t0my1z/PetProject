FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    ninja-build \
    gdb \
    valgrind \
    git \
    git-lfs \
    dbus-x11 \
    gnome-terminal \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*