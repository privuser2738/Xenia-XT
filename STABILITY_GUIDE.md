# Xenia Stability Guide

This guide helps reduce random crashes during gameplay.

## Quick Fix: Use Stable Configuration

Copy `portable.txt` to the same directory as `xenia.exe`. This file contains settings optimized for stability.

**Location:**
```
build\bin\Windows\Release\portable.txt
```

## Why Random Crashes Happen

Xenia is an Xbox 360 emulator that translates console code to run on PC. Random crashes can occur from:

1. **Unimplemented Xbox 360 functions** - Some games use features not yet emulated
2. **Race conditions** - Timing issues in multi-threaded emulation
3. **Memory access violations** - Games accessing memory incorrectly
4. **GPU emulation bugs** - Graphics code that doesn't translate perfectly

## Stability Tips

### 1. Use Release Build (Not Debug)
Release builds are **much more stable**:
```cmd
xb.cmd build --config Release
build\bin\Windows\Release\xenia.exe
```

### 2. Keep Saves Regularly
Crashes can happen anytime, so save your game progress often.

### 3. Try Different GPU Backends
If you get graphics-related crashes, try switching:

**D3D12 (Default):**
```
--gpu=d3d12
```

**Vulkan (Alternative):**
```
--gpu=vulkan
```

Example:
```cmd
xenia.exe --gpu=vulkan game.iso
```

### 4. Disable Problematic Features

Some features can cause instability:

**Disable Discord Rich Presence:**
```
--discord=false
```

**Disable CPU optimizations:**
```
--cpu_unsafe_optimizations=false
```

### 5. Use Lower Settings

**Disable VSync if causing issues:**
```
--vsync=false
```

**Lower resolution scale:**
```
--gpu_resolution_scale=1
```

## Configuration File

Create a `portable.txt` file in the xenia.exe directory:

```
# CPU Stability
cpu_unsafe_optimizations = false

# Memory Protection
protect_zero = false
protect_on_release = false

# GPU Settings
gpu = d3d12
gpu_allow_invalid_fetch_constants = true

# General
vsync = true
ignore_thread_affinities = true
ignore_thread_priorities = true

# Logging (to debug crashes)
log_file = xenia.log
log_level = 2
```

## Debugging Crashes

If crashes persist, enable logging to find the cause:

1. Add to `portable.txt`:
   ```
   log_file = xenia.log
   log_level = 3
   ```

2. Run the game until it crashes

3. Check `xenia.log` for error messages before the crash

4. The last few lines often show what went wrong

## Common Crash Causes

### Audio Crashes
**Symptom:** Crashes during cutscenes or audio-heavy scenes
**Fix:** Try different audio backends:
```cmd
xenia.exe --apu=nop game.iso    # Disable audio
xenia.exe --apu=sdl game.iso    # Use SDL audio
```

### GPU Crashes
**Symptom:** Crashes during specific graphical effects
**Fix:**
- Try Vulkan: `--gpu=vulkan`
- Lower resolution: `--gpu_resolution_scale=1`
- Update graphics drivers

### Memory Crashes
**Symptom:** Random crashes after playing for a while
**Fix:**
- Restart Xenia every hour or so
- Close other programs to free memory
- Use 64-bit Release build (smaller memory footprint)

## Known Limitations

- Some games are more stable than others
- Not all Xbox 360 features are implemented
- Crashes are part of emulation development
- Report persistent crashes to Xenia project on GitHub

## Performance vs Stability

**More Stable (Slower):**
```
cpu_unsafe_optimizations = false
vsync = true
gpu_resolution_scale = 1
```

**Less Stable (Faster):**
```
cpu_unsafe_optimizations = true
vsync = false
gpu_resolution_scale = 3
```

Choose based on your priorities!

## If All Else Fails

1. Try official Xenia builds from xenia.jp (might be more/less stable)
2. Check game compatibility list
3. Report the crash with logs to Xenia project
4. Some games just aren't stable yet - try a different game

## Useful Command Line Options

```cmd
# Full stability mode
xenia.exe --cpu_unsafe_optimizations=false --protect_zero=false --vsync=true game.iso

# Debug mode with logging
xenia.exe --log_file=crash.log --log_level=3 game.iso

# Minimal mode (disable everything)
xenia.exe --gpu=d3d12 --apu=nop --discord=false game.iso
```

Good luck and happy gaming! 🎮
