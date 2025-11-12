# Building Xenia Without Python/npm

This guide explains how to build Xenia using only native Windows tools, without requiring Python or npm.

## Prerequisites

* Windows 7 or later
* [Visual Studio 2022, Visual Studio 2019, or Visual Studio 2017](https://www.visualstudio.com/downloads/)
  * For Visual Studio 2022, MSBuild `v142` must be used due to a compiler bug
* Windows 11 SDK version 10.0.22000.0 (or any newer version for VS2019)
* Git (optional, for version info and submodule management)

**Note:** Python and npm are NO LONGER required with this native build system!

## Quick Start

```cmd
REM Clone the repository
git clone https://github.com/xenia-project/xenia.git
cd xenia

REM Setup (initialize submodules and run premake)
xb.cmd setup

REM Build (default is Debug configuration)
xb.cmd build

REM Build Release configuration
xb.cmd build --config Release
```

### Custom MSBuild Location

If you have msbuild in a non-standard location, you can set the `MSBUILD` environment variable:

```cmd
SET MSBUILD=C:\path\to\msbuild.exe
xb.cmd build
```

The script automatically checks these locations in order:
1. `MSBUILD` environment variable
2. `C:\c\msbuild.exe` (common standalone location)
3. System PATH
4. Visual Studio installation (via VsDevCmd.bat)
5. Standard VS 2017/2019/2022 installation paths

## Available Commands

### `xb.cmd setup`
Initializes git submodules and runs premake to generate Visual Studio solution files.

```cmd
xb.cmd setup
```

### `xb.cmd build`
Builds the project using MSBuild.

```cmd
REM Build debug configuration (default)
xb.cmd build

REM Build release configuration
xb.cmd build --config Release

REM Build specific target
xb.cmd build --target xenia-app

REM Force rebuild (clean + build)
xb.cmd build --force

REM Build without running premake first
xb.cmd build --no_premake
```

Configuration options:
* `Debug` - Debug build with symbols
* `Release` - Optimized release build
* `Checked` - Debug build with runtime checks

### `xb.cmd premake`
Runs premake to regenerate Visual Studio project files.

```cmd
xb.cmd premake
```

### `xb.cmd devenv`
Runs premake and then launches Visual Studio with the solution.

```cmd
xb.cmd devenv
```

### `xb.cmd clean`
Removes all build artifacts and the build directory.

```cmd
xb.cmd clean
```

### `xb.cmd pull`
Updates the repository and submodules from git, then runs premake.

```cmd
xb.cmd pull
```

## Manual Building

If you prefer to build manually:

1. **Setup submodules** (if you have git):
   ```cmd
   git submodule update --init
   ```

2. **Run premake** to generate Visual Studio files:
   ```cmd
   tools\build\bin\premake5.exe --scripts=third_party\premake-core --file=premake5.lua --os=windows --test-suite-mode=combined vs2022
   ```

3. **Build with MSBuild**:
   ```cmd
   msbuild build\xenia.sln /nologo /m /v:m /p:Configuration=Release
   ```

4. **Or open in Visual Studio**:
   ```cmd
   build\xenia.sln
   ```

## What Changed?

The original build system (`xb.bat` and `xenia-build`) required Python 3.6+. This new native build system:

* ✅ **No Python required** - Pure Windows batch scripts
* ✅ **No npm required** - Not needed for core builds
* ✅ **Faster** - Direct execution without Python interpreter overhead
* ✅ **Simpler** - Fewer dependencies to install
* ✅ **Compatible** - Works with same premake and Visual Studio setup

## Troubleshooting

### "premake5.exe not found"
Make sure you've initialized the git submodules:
```cmd
git submodule update --init
```

Or manually download premake5.exe and place it in `tools\build\bin\`

### "msbuild not found"
The build script automatically detects and configures Visual Studio. If you get this error:

1. **Make sure Visual Studio is installed** (2017, 2019, or 2022 with C++ development tools)
2. **Run from a Visual Studio Developer Command Prompt** as a fallback
3. **Check that vswhere.exe exists** in `tools\vswhere\vswhere.exe`

The script tries to:
- Detect Visual Studio using vswhere.exe
- Automatically call VsDevCmd.bat or vcvarsall.bat
- Set up the build environment for you

If automatic detection fails, you can manually run from a "Developer Command Prompt for VS":
```cmd
# Open "Developer Command Prompt for VS 2022" from Start Menu
cd C:\path\to\xenia
xb.cmd build
```

### "Microsoft.Cpp.Default.props not found"
This means the Visual Studio environment isn't properly initialized. Solutions:

1. Run the script from a **Visual Studio Developer Command Prompt**
2. Make sure Visual Studio Build Tools are installed
3. Verify that `tools\vswhere\vswhere.exe` exists

### Build errors
1. Make sure you have the Windows SDK installed (version 10.0.22000.0 or newer)
2. Try cleaning and rebuilding:
   ```cmd
   xb.cmd clean
   xb.cmd setup
   xb.cmd build
   ```
3. If all else fails, try from a Visual Studio Developer Command Prompt

## Legacy System

The original Python-based build system is still available:
* `xb.bat` - Original Python wrapper (requires Python 3.6+)
* `xenia-build` - Original Python build script

If you need to use the old system, just use `xb.bat` instead of `xb.cmd`.
