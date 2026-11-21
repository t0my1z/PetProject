# PetProject

PetProject is a C++23 example of a modular network application. The `comms_manager` executable owns network connections and loads dynamic libraries called modules at runtime. The included Chat Creator and Chat Joiner modules demonstrate the design with a small turn-based chat example; the project is intended to demonstrate the communication and module architecture, not to be a full chat application.

## How it works

1. `comms_manager` reads a `config.yaml` from a configuration directory supplied with `--config_folder`.
2. It creates the configured UDP or TCP connections and loads each configured module from the module directory.
3. Modules exchange data through the manager's connection and protocol interfaces.

If `--module_folder` is omitted, the manager looks for modules in the `lib/modules` directory next to the installed configuration directories. The supplied install layout follows this convention.

The sample creator and joiner configurations use UDP over `127.0.0.1`, with ports `9990` and `9991`. TCP connection support is also implemented.

## Dependencies

- CMake 3.23 or newer.
- A C++23-capable compiler and the build tools for the selected CMake generator.
- The project vendors its dependencies, so separate Asio or yaml-cpp installations are not required:
  - Standalone Asio is header-only and compiled into the executable.
  - yaml-cpp is built as a static library by default and linked into the executable.
- On Windows, CMake also links the required Winsock system libraries.

CMake uses the generator selected by the local environment.

## Compile and install

Run these commands from the repository root. The install prefix is split by platform, producing `install/windows` on Windows and `install/linux` on Linux.

### Windows (PowerShell)

Use a C++23-capable Visual Studio/MSVC installation and the Windows SDK:

```powershell
cmake -S . -B build
cmake --build build --config Release --parallel
cmake --install build --prefix ./install --config Release
```

### Linux

Use a C++23-capable GCC or Clang toolchain and a supported CMake generator:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix ./install
```

### Docker on Linux

Docker provides an Ubuntu-based Linux build environment containing the compiler and build tools required by PetProject. Docker is especially useful when the host system does not already have a C++23 toolchain.

Docker Engine must be installed and running. From the repository root, build the development image:

```bash
docker build -t petproject-dev .
```

Then start an interactive container and mount the repository into it:

```bash
docker run --rm -it --name petproject \
  -v "$(pwd):/workspaces/PetProject" \
  -w /workspaces/PetProject \
  petproject-dev bash
```

Inside the container, use the Linux build script:

```bash
bash scripts/linux/build.sh
```

The build and installed files are written to `build/` and `install/` in the repository, because the container has mounted the repository directory.

To run the sample with one container, open another Linux terminal and start the Chat Creator in the container:

```bash
docker exec -it petproject bash -lc \
  './install/linux/bin/comms_manager --config_folder ./install/linux/config'
```

Open a third terminal and start the Chat Joiner:

```bash
docker exec -it petproject bash -lc \
  './install/linux/bin/comms_manager --config_folder ./install/linux/config_joiner'
```

Both processes run in the same container network namespace, so their loopback UDP connections on ports `9990` and `9991` can communicate. Stop the processes with `Ctrl+C`. Exit the original container with `exit`; `--rm` removes the container automatically.

### Build scripts

The build scripts perform a clean build by deleting the repository's `build` and `install` directories first. Back up anything in those directories that you need to keep.

- Windows: run `scripts\windows\build.bat`.
- Linux: run `bash scripts/linux/build.sh`.

The Linux build and run scripts are provided.

## Run the sample

The run scripts start the Chat Creator first and then the Chat Joiner, each in a separate terminal window. Build and install the project before running them.

- Windows: run `scripts\windows\run_chat.bat`.
- Linux: run `bash scripts/linux/run_chat.sh`.

To start an instance manually, pass a configuration directory containing `config.yaml`:

```powershell
.\install\windows\bin\comms_manager.exe --config_folder .\install\windows\config
```

```bash
./install/linux/bin/comms_manager --config_folder ./install/linux/config
```

Use the corresponding `config_joiner` directory to start the Joiner. Start each instance in its own terminal. You can also pass `--module_folder <path>` if your modules are installed outside the default `lib/modules` location.

## Installation layout

CMake installs the executable, configuration directories, public headers, and sample modules under the selected prefix. For example:

```text
install/
  windows/
    bin/comms_manager.exe
    config/config.yaml
    config_joiner/config.yaml
    lib/modules/
  linux/
    bin/comms_manager
    config/config.yaml
    config_joiner/config.yaml
    lib/modules/
```
