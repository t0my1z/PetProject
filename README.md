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

### Build scripts

The scripts perform a clean build by deleting the repository's `build` and `install` directories first. Back up anything in those directories that you need to keep.

- Windows: run `scripts\windows\build.bat`.
- Linux: run `bash scripts/linux/build.sh`.

The Linux build and run scripts are provided, but Linux has not yet been verified in the project environment.

## Run the sample

The run scripts start the Chat Creator first and then the Chat Joiner, each in a separate terminal window. Build and install the project before running them.

- Windows: run `scripts\windows\run_chat.bat`.
- Linux: run `bash scripts/linux/run_chat.sh`. A supported graphical terminal emulator (`x-terminal-emulator`, GNOME Terminal, Konsole, Xfce Terminal, or xterm) is required.

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
