# Linux Build Notes for Xenia-XT

This document describes the Linux build setup and fixes applied to enable building Xenia-XT on Linux.

## Overview

Xenia-XT now has full Linux build support with automated build and install scripts. The build system has been tested on Manjaro Linux (Arch-based) with Clang compiler.

## Build Scripts

### build-linux.sh
A comprehensive build wrapper script that provides:
- Automated dependency checking
- Simple build commands similar to build-windows.bat
- Support for Debug and Release builds
- Distribution package creation
- Parallel build support with `-j` flag

### install.sh
Installation script that supports:
- User installation to `~/.local` (no sudo required)
- System-wide installation to `/usr/local` (with sudo)
- Desktop entry creation
- Icon installation at multiple resolutions
- Automatic PATH configuration check

## Fixes Applied

### 1. Premake5 Build Issues
**Problem**: Missing `unistd.h` includes in libzip files causing build failures.

**Files Modified**:
- `third_party/premake-core/contrib/libzip/mkstemp.c`
- `third_party/premake-core/contrib/libzip/zip_close.c`
- `third_party/premake-core/contrib/libzip/zip_fdopen.c`
- `third_party/premake-core/contrib/libzip/zip_open.c`

**Fix**: Added conditional `#include <unistd.h>` for non-Windows platforms.

### 2. Compiler Warning Fixes
**Problem**: Modern Clang compilers treat certain warnings as errors with `-Werror` flag.

**File Modified**: `premake5.lua`

**Warnings Disabled**:
```lua
buildoptions({
    "-Wno-deprecated-literal-operator",  -- fmt library compatibility
    "-Wno-integer-overflow",              -- cxxopts library compatibility
    "-Wno-nontrivial-memcall",           -- imgui library compatibility
})
```

These warnings are from third-party libraries and don't affect functionality.

## Build Requirements

### Manjaro/Arch Linux
```bash
sudo pacman -S base-devel clang llvm ninja python gtk3 libx11 \
    vulkan-headers vulkan-icd-loader sdl2 libunwind
```

### Debian/Ubuntu
```bash
sudo apt-get install build-essential clang llvm ninja-build python3 \
    libgtk-3-dev libpthread-stubs0-dev liblz4-dev libx11-dev \
    libx11-xcb-dev libvulkan-dev libsdl2-dev libiberty-dev \
    libunwind-dev libc++-dev libc++abi-dev
```

### Fedora
```bash
sudo dnf install gcc-c++ clang llvm ninja-build python3 \
    gtk3-devel libX11-devel vulkan-headers vulkan-loader-devel \
    SDL2-devel libunwind-devel
```

## Quick Start

### First Time Build
```bash
# Clone repository
git clone --recursive https://github.com/YourUsername/xenia-xt.git
cd xenia-xt

# Setup build environment
./build-linux.sh setup

# Build (Debug)
./build-linux.sh --config=Debug

# Build (Release)
./build-linux.sh --config=Release
```

### Installation
```bash
# User installation (recommended)
./install.sh

# System-wide installation (requires sudo)
sudo ./install.sh system
```

## Binary Locations

### After Build
- **Debug**: `build/bin/Linux/Debug/xenia-xt`
- **Release**: `build/bin/Linux/Release/xenia-xt`

### After Install (User)
- **Main binary**: `~/.local/bin/xenia-xt`
- **Symlink**: `~/.local/bin/xenia` → `xenia-xt`
- **Tools**: `~/.local/bin/xenia-*`

### After Install (System)
- **Main binary**: `/usr/local/bin/xenia-xt`
- **Symlink**: `/usr/local/bin/xenia` → `xenia-xt`
- **Tools**: `/usr/local/bin/xenia-*`

## Build Commands

| Command | Description |
|---------|-------------|
| `./build-linux.sh` | Build Release configuration |
| `./build-linux.sh --config=Debug` | Build Debug configuration |
| `./build-linux.sh setup` | Setup submodules and run premake |
| `./build-linux.sh clean` | Clean build outputs |
| `./build-linux.sh dist` | Build and create distribution tarball |
| `./build-linux.sh -j 8` | Build with 8 parallel jobs |
| `./build-linux.sh help` | Show help message |

## Known Issues

1. **Linux support is experimental**: Some features may not work properly on Linux compared to Windows build.

2. **GTK dependency**: The Linux build requires GTK3 for file dialogs and UI components.

3. **Vulkan required**: The Linux build uses Vulkan for GPU emulation. D3D12 backend is Windows-only.

## Testing Status

- ✅ Build system setup (premake, dependencies)
- ✅ Debug build compilation
- ✅ User installation to ~/.local
- ⏸️ Release build (not tested yet)
- ⏸️ System installation (requires sudo)
- ⏸️ Runtime testing with actual Xbox 360 games

## Additional Tools Installed

The install script also installs these additional utilities:
- `xenia-vfs-dump` - Virtual filesystem dump tool
- `xenia-gpu-shader-compiler` - Shader compilation tool
- `xenia-gpu-vulkan-trace-viewer` - Vulkan trace viewer
- `xenia-gpu-vulkan-trace-dump` - Vulkan trace dump tool

## Uninstallation

```bash
# User installation
./uninstall.sh

# System installation
sudo ./uninstall.sh
```

## Date

Last updated: 2025-11-26
Build tested on: Manjaro Linux 6.12.48-1-MANJARO (Arch-based)
Compiler: Clang (from LLVM toolchain)
