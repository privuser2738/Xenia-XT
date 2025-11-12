# Xenia Build Guide

## Quick Start

### Using the Intelligent Build Script (Recommended)

Simply run:
```cmd
build.bat
```

The script will automatically:
- ✅ Detect if a rebuild is needed by checking source file timestamps
- ✅ Skip rebuilding if nothing has changed (saves time!)
- ✅ Perform incremental builds when only source files changed
- ✅ Perform full builds when project structure changed
- ✅ Show detailed analysis of what changed

### Build Output

**When up-to-date:**
```
[SUCCESS] No rebuild needed - all sources are up to date!
```

**When rebuilding:**
```
[REBUILD] Rebuild required
Reason: Source file(s) modified
Changed files: src\xenia\base\crash_recovery.cc
```

## Manual Build Commands

### Full Build (with premake)
```cmd
xb.cmd build
```

### Incremental Build (skip premake)
```cmd
xb.cmd build --no_premake
```

### Clean Build
```cmd
xb.cmd clean
build.bat
```

## Build Artifacts

After a successful build, you'll find:
- Executable: `build\bin\Windows\Debug\xenia.exe` (~25 MB)
- Crash Database: `build\bin\Windows\Debug\crash_recovery.db` (created on first crash)
- Log files: Various `.log` files in the output directory

## Running Xenia with Logging

### Using the Logging Script (Recommended)
```cmd
cd build\bin\Windows\Debug
run_with_log.cmd "path\to\game.xex"
```

This will:
- Run xenia with full logging enabled
- Save output to `xenia_log.txt`
- Display the log after xenia exits
- Show crash recovery database if crashes occurred

### Direct Execution
```cmd
cd build\bin\Windows\Debug
xenia.exe
```

## Crash Recovery System

The build includes an intelligent crash recovery system that:
- 🛡️ Catches crashes and logs details
- 📊 Tracks crash frequency and patterns
- 🔧 Automatically applies workarounds for repeated crashes
- 💾 Saves learning data to `crash_recovery.db`

### Viewing Crash Data

After a crash, check:
1. Console output for real-time crash notifications
2. `crash_recovery.db` for persistent crash history
3. `xenia_log.txt` (if using `run_with_log.cmd`)

## Recent Fixes

### Memory Management (xboxkrnl_memory.cc)
- ✅ Fixed null pointer crashes in heap lookups
- ✅ Added protection for NtAllocateVirtualMemory
- ✅ Added protection for NtProtectVirtualMemory
- ✅ Added protection for NtFreeVirtualMemory

### Build System
- ✅ Fixed MSBuild configuration (Debug|Windows)
- ✅ Created intelligent incremental build system

### Crash Recovery
- ✅ Initialized crash handlers on startup
- ✅ Auto-learning workaround system
- ✅ Crash frequency tracking

## Troubleshooting

### "Build failed" error
1. Check that Visual Studio 2022 is installed with C++ tools
2. Try a clean build: `xb.cmd clean && build.bat`
3. Check the error messages in the build output

### Xenia crashes immediately
1. Check `xenia_log.txt` or console output
2. Look for error messages starting with `e>`
3. Check `crash_recovery.db` for crash patterns
4. Try running with logging: `run_with_log.cmd`

### "No rebuild needed" but changes not reflected
- The build system checks timestamps
- Make sure you saved your source files
- If in doubt, touch the file: `touch src/path/to/file.cc`
- Or do a clean build: `xb.cmd clean && build.bat`

## Build Time Estimates

- **Initial full build**: ~3-5 minutes (depends on hardware)
- **Incremental build**: ~30-90 seconds (only changed files)
- **No changes detected**: <1 second (instant)

## Tips

1. **Use `build.bat` for daily development** - it's smart about what needs rebuilding
2. **Use `run_with_log.cmd`** when testing games - captures all output
3. **Check crash_recovery.db** after crashes - it learns and adapts
4. **Do a full rebuild** after pulling updates: `xb.cmd clean && build.bat`

## Questions?

For build issues, check:
- This guide
- Build output messages
- `xenia_log.txt` for runtime issues
- GitHub issues: https://github.com/xenia-project/xenia/issues
