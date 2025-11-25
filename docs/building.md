# Building Xenia-XT

You must have a 64-bit machine for building and running the project. Always
run your system updater before building and make sure you have the latest
drivers.

## Quick Start (Windows)

```batch
# Clone with submodules
git clone --recursive https://github.com/YourUsername/xenia-xt.git
cd xenia-xt

# Build Release (recommended)
build-windows.bat

# Output: build\bin\Windows\Release\xenia-xt.exe
```

## Setup

### Windows

#### Requirements

* Windows 10 or Windows 11 (64-bit)
* [Visual Studio 2022 or Visual Studio 2019](https://www.visualstudio.com/downloads/)
  * For Visual Studio 2022, MSBuild `v142` must be used due to a compiler bug
* [Python 3.6+](https://www.python.org/downloads/)
  * Ensure Python is in PATH
* Windows SDK version 10.0.22000.0 or newer

#### Build Commands

Xenia-XT uses `build-windows.bat` as the primary build script:

| Command | Description |
|---------|-------------|
| `build-windows.bat` | Build Release configuration (default) |
| `build-windows.bat --config=Debug` | Build Debug configuration |
| `build-windows.bat setup` | Setup submodules and regenerate project files |
| `build-windows.bat clean` | Clean all build outputs |
| `build-windows.bat dist` | Build Release and create distribution package |
| `build-windows.bat help` | Show all available options |

#### Examples

```batch
# First time setup
git clone --recursive https://github.com/YourUsername/xenia-xt.git
cd xenia-xt
build-windows.bat setup

# Regular build
build-windows.bat

# Debug build
build-windows.bat --config=Debug

# Create distribution package (in dist/ folder)
build-windows.bat dist
```

#### Advanced Build (xb.cmd)

For more granular control, use `xb.cmd` directly:

```batch
# Setup (first time only)
xb.cmd setup

# Build
xb.cmd build                      # Debug build
xb.cmd build --config Release     # Release build

# Update after pulling changes
xb.cmd pull

# Open in Visual Studio
xb.cmd devenv

# Regenerate project files
xb.cmd premake

# Format code
xb.cmd format
```

#### Debugging in Visual Studio

1. Open the solution via `xb.cmd devenv`
2. Set `xenia-app` as the startup project
3. Open project properties and set:
   - **Command**: `$(SolutionDir)$(TargetPath)`
   - **Working Directory**: `$(SolutionDir)..\..`
   - **Command Arguments**: Path to game file (e.g., `C:\Games\game.iso`)

You can also use `--flagfile=flags.txt` to specify flags and use
`--log_file=log.txt` to override the default log location.

For JIT debugging, pass `--emit_source_annotations` to get helpful
annotations in the disassembly around address 0xA0000000.

### Linux

Linux support is experimental and incomplete.

#### Requirements

* LLVM/Clang 9+
* Development libraries:

```bash
sudo apt-get install libgtk-3-dev libpthread-stubs0-dev liblz4-dev \
    libx11-dev libx11-xcb-dev libvulkan-dev libsdl2-dev libiberty-dev \
    libunwind-dev libc++-dev libc++abi-dev
```

* Vulkan drivers for your GPU

#### Build

```bash
# Build with Make
xb build

# Or use CodeLite
xb devenv

# CMake (experimental, for CLion)
xb premake --devenv=cmake
```

## Running

### Command Line

```batch
# Run with a game
xenia-xt.exe C:\Games\game.iso

# Run with logging to console
xenia-xt.exe --log_file=stdout C:\Games\game.iso

# Run with specific GPU backend
xenia-xt.exe --gpu=vulkan C:\Games\game.iso
xenia-xt.exe --gpu=d3d12 C:\Games\game.iso
```

### Configuration File

Xenia-XT stores its configuration in:
- `Documents\Xenia-XT\xenia-xt.config.toml`

You can edit this file to change settings like GPU backend, audio settings, etc.

## Troubleshooting

### Build Errors

1. **Missing Python**: Ensure Python 3.6+ is installed and in PATH
2. **Missing Visual Studio**: Install VS 2019 or 2022 with C++ workload
3. **Missing SDK**: Install Windows SDK 10.0.22000.0 or newer
4. **Submodule errors**: Run `git submodule update --init --recursive`

### Runtime Errors

1. **GPU errors**: Try switching between D3D12 and Vulkan in config
2. **Missing DLLs**: Ensure Visual C++ Redistributable is installed
3. **Crashes on game load**: Check the log file for details

## Project Structure

```
xenia-xt/
├── build-windows.bat    # Main build script
├── xb.cmd               # Advanced build script
├── src/
│   └── xenia/
│       ├── app/         # Application/UI code
│       ├── base/        # Base utilities
│       ├── cpu/         # CPU emulation
│       ├── gpu/         # GPU emulation
│       ├── kernel/      # Xbox kernel emulation
│       └── ui/          # UI framework
├── docs/                # Documentation
├── third_party/         # Third-party dependencies
└── build/               # Build outputs (generated)
```
