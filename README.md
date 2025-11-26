<p align="center">
    <a href="https://github.com/xenia-project/xenia/tree/master/assets/icon">
        <img height="120px" src="https://raw.githubusercontent.com/xenia-project/xenia/master/assets/icon/128.png" />
    </a>
</p>

<h1 align="center">Xenia-XT - Xbox 360 Emulator</h1>

Xenia-XT is an enhanced fork of the Xenia Xbox 360 emulator, featuring improved
compatibility, additional fixes, and quality-of-life enhancements. For the
original project, see [Xenia](https://github.com/xenia-project/xenia).

## Features

Xenia-XT includes several enhancements over the base Xenia emulator:

- **Improved Game Compatibility** - Additional fixes for games that don't work in standard Xenia
- **Network Bypass Stubs** - Games requiring Xbox Live initialization can now boot in offline mode
- **Crash Recovery System** - Automatic detection and workarounds for known crash points
- **Game Compatibility Database** - Built-in database of game-specific fixes
- **Enhanced UI** - Close Game menu option (Ctrl+W), improved file handling
- **Better Stability** - Safe pause/resume, improved title switching

## Status

Xenia-XT is based on the latest Xenia codebase with additional enhancements.
Many games that have issues in standard Xenia may work better in Xenia-XT.

See the [Game compatibility list](https://github.com/xenia-project/game-compatibility/issues)
for tracked games from the main project.

## Disclaimer

The goal of this project is to experiment, research, and educate on the topic
of emulation of modern devices and operating systems. **It is not for enabling
illegal activity**. All information is obtained via reverse engineering of
legally purchased devices and games and information made public on the internet.

## Quickstart

### Windows

1. Download the latest release or build from source
2. Run `xenia-xt.exe`
3. Use File → Open to load an Xbox 360 game (ISO, XEX, or extracted folder)

### Linux

1. Build from source (see Building section below)
2. Run `./build/bin/Linux/Release/xenia` or install with `./install.sh`
3. Use File → Open to load an Xbox 360 game (ISO, XEX, or extracted folder)

### Controls

- **Ctrl+O** - Open game file
- **Ctrl+W** - Close current game
- **F11** - Toggle fullscreen
- **F3** - Toggle profiler display

## Building

### Windows

#### Requirements

- Windows 10/11 (64-bit)
- [Visual Studio 2022 or 2019](https://www.visualstudio.com/downloads/)
- [Python 3.6+](https://www.python.org/downloads/) (ensure it's in PATH)
- Windows SDK (10.0.22000.0 or newer)

#### Quick Build

```batch
# Clone the repository
git clone --recursive https://github.com/YourUsername/xenia-xt.git
cd xenia-xt

# Build (creates xenia-xt.exe in build\bin\Windows\Release\)
build-windows.bat

# Or build with options
build-windows.bat --config=Debug
build-windows.bat dist              # Build and create distribution package
build-windows.bat help              # Show all options
```

#### Build Commands

| Command | Description |
|---------|-------------|
| `build-windows.bat` | Build Release configuration |
| `build-windows.bat --config=Debug` | Build Debug configuration |
| `build-windows.bat setup` | Setup submodules and run premake |
| `build-windows.bat clean` | Clean build outputs |
| `build-windows.bat dist` | Build and create distribution in `dist/` folder |
| `build-windows.bat help` | Show help message |

#### Advanced Build (using xb.cmd)

For more advanced build options, use the `xb.cmd` script directly:

```batch
xb.cmd setup              # Initial setup
xb.cmd build              # Build debug
xb.cmd build --config Release  # Build release
xb.cmd premake            # Regenerate project files
xb.cmd devenv             # Open in Visual Studio
xb.cmd format             # Format code
```

### Linux

#### Requirements

- 64-bit Linux distribution
- LLVM/Clang 9+ or GCC 9+
- [Python 3.6+](https://www.python.org/downloads/)
- Build tools: make or ninja
- Development libraries (install before building):

**Debian/Ubuntu:**
```bash
sudo apt-get install build-essential clang llvm ninja-build python3 \
    libgtk-3-dev libpthread-stubs0-dev liblz4-dev libx11-dev \
    libx11-xcb-dev libvulkan-dev libsdl2-dev libiberty-dev \
    libunwind-dev libc++-dev libc++abi-dev
```

**Arch/Manjaro:**
```bash
sudo pacman -S base-devel clang llvm ninja python gtk3 libx11 \
    vulkan-headers vulkan-icd-loader sdl2 libunwind
```

**Fedora:**
```bash
sudo dnf install gcc-c++ clang llvm ninja-build python3 \
    gtk3-devel libX11-devel vulkan-headers vulkan-loader-devel \
    SDL2-devel libunwind-devel
```

#### Quick Build

```bash
# Clone the repository
git clone --recursive https://github.com/YourUsername/xenia-xt.git
cd xenia-xt

# First-time setup
./build-linux.sh setup

# Build (creates xenia in build/bin/Linux/Release/)
./build-linux.sh

# Or build with options
./build-linux.sh --config=Debug
./build-linux.sh dist              # Build and create distribution package
./build-linux.sh help              # Show all options
```

#### Build Commands

| Command | Description |
|---------|-------------|
| `./build-linux.sh` | Build Release configuration |
| `./build-linux.sh --config=Debug` | Build Debug configuration |
| `./build-linux.sh setup` | Setup submodules and run premake |
| `./build-linux.sh clean` | Clean build outputs |
| `./build-linux.sh dist` | Build and create distribution tarball |
| `./build-linux.sh help` | Show help message |

#### Installation

After building, you can install Xenia system-wide or for the current user:

```bash
# Install for current user (no sudo required)
./install.sh

# Install system-wide (requires sudo)
sudo ./install.sh system

# Uninstall
./uninstall.sh        # User install
sudo ./uninstall.sh   # System install
```

#### Advanced Build (using xenia-build)

For more advanced build options, use the `xenia-build` script directly:

```bash
./xenia-build setup              # Initial setup
./xenia-build build              # Build debug
./xenia-build build --config release  # Build release
./xenia-build premake            # Regenerate project files
./xenia-build clean              # Clean outputs
```

See [docs/building.md](docs/building.md) for detailed build instructions.

## Configuration

Xenia-XT stores configuration in:
- Windows: `Documents\Xenia-XT\xenia-xt.config.toml`

Common configuration options:

```toml
[GPU]
gpu = "any"  # Options: any, d3d12, vulkan, null

[Display]
fullscreen = false
```

## Contributing

Contributions are welcome! Please check:

- [Style Guide](docs/style_guide.md) for code formatting
- Run `xb.cmd format` before submitting PRs

## License

Xenia-XT is licensed under the BSD license. See [LICENSE](LICENSE) for details.

## Credits

- [Xenia Project](https://github.com/xenia-project/xenia) - Original emulator
- All contributors to the Xenia project

## Links

- [Original Xenia Wiki](https://github.com/xenia-project/xenia/wiki)
- [Original Xenia FAQ](https://github.com/xenia-project/xenia/wiki/FAQ)
- [Game Compatibility List](https://github.com/xenia-project/game-compatibility/issues)
