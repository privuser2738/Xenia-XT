# Xenia Game Compatibility System

## Overview

Xenia now includes an intelligent game compatibility and crash recovery system that learns from known issues and automatically applies fixes to improve game compatibility. The system prevents crashes before they happen by applying game-specific workarounds.

## Features

### 1. Automatic Crash Recovery
- **Runtime crash detection**: Catches crashes before they terminate the emulator
- **Self-learning system**: Records crash patterns and frequencies
- **Automatic workarounds**: Applies fixes after repeated crashes
- **Persistent database**: Saves learned crash data between sessions

### 2. Game Compatibility Database
- **Pre-configured fixes**: Built-in workarounds for 30+ popular Xbox 360 games
- **Game-specific settings**: Memory, graphics, and CPU configurations per title
- **Compatibility ratings**: Track which games work and how well
- **Community updates**: Designed to receive updates from online databases

### 3. Proactive Error Prevention
- **Memory configuration**: Pre-allocates optimal memory for problematic games
- **Graphics workarounds**: Applies GPU-specific fixes for known issues
- **CPU blacklisting**: Skips problematic code addresses automatically
- **Zero user intervention**: Everything happens automatically

## Currently Supported Games

The system includes optimized configurations for:

### **Perfect/Playable Games**
- Halo 3, Halo: Reach, Halo 4
- Gears of War series (1-3)
- Call of Duty: Modern Warfare 2, Black Ops
- Fable II & III
- Mass Effect series (1-3)
- Batman: Arkham Asylum & City
- Bioshock, Portal 2, Left 4 Dead
- Minecraft: Xbox 360 Edition
- Beautiful Katamari, Crackdown
- Mirror's Edge, Dead Space
- Assassin's Creed II

### **Playable with Issues**
- GTA IV & V
- Red Dead Redemption
- Skyrim
- Fallout 3 & New Vegas
- Forza Motorsport 3 & 4
- Saints Row 2 & The Third
- Alan Wake
- Bioshock Infinite

## How It Works

### Automatic Fix Application

When you load a game, Xenia automatically:

1. **Identifies the game** by its Title ID
2. **Looks up compatibility data** from the built-in database
3. **Applies known fixes** before the game starts
4. **Monitors for crashes** during gameplay
5. **Learns from issues** and applies new workarounds

Example log output:
```
========================================
GAME COMPATIBILITY INFO
  Title: Halo 3
  Status: 3 (Playable)
  Known Issues: 1
  Available Fixes: 2
========================================
Applying 2 fixes for title 4D5307E6
  - Applying: Fix graphics flickering on Ampere GPUs
```

### Crash Recovery

If a crash occurs:

1. **Crash is caught** by the exception handler
2. **Details are logged** (type, address, frequency)
3. **Workaround is determined** based on crash type
4. **Fix is applied automatically** after 3 occurrences
5. **Data is saved** for future runs

## Common Fixes Applied

### Memory Configuration
- **Heap size adjustments**: Prevents memory exhaustion crashes
- **Large page support**: Improves performance for streaming-heavy games
- **Reserved regions**: Protects critical memory areas

### Graphics Settings
- **Safe shader cache**: Prevents shader compilation crashes
- **Render cache control**: Fixes flickering on certain GPUs
- **Texture size limits**: Prevents VRAM exhaustion
- **MSAA/Tessellation**: Disables problematic features per game

### CPU Workarounds
- **Address blacklisting**: Skips known problematic code
- **Safe JIT mode**: Uses safer compilation for unstable games
- **Code patching**: Fixes specific instruction sequences
- **Function skipping**: Bypasses broken game functions

## Adding New Games

### For Developers

To add support for a new game, edit `src/xenia/base/game_compatibility.cc`:

```cpp
// In InitializeBuiltInDatabase():
RegisterKnownGame(0xTITLEID, "Game Name", CompatibilityStatus::Playable);

// In AddBuiltInFixes():
if (HasGameInfo(0xTITLEID)) {
  GameFix fix;
  fix.type = FixType::MemoryConfiguration;
  fix.description = "Prevent memory crashes";
  fix.enabled = true;
  fix.priority = 10;
  fix.memory_config.heap_size_64kb = 8192;  // 512MB
  fix.memory_config.use_large_pages = true;
  AddFix(0xTITLEID, fix);
}
```

### Finding Title IDs

Title IDs are displayed in Xenia's log when loading a game:
```
Launching module game:\default.xex
Title ID: 4D5307E6
```

You can also find them at:
- **Xbox.com** - Look in game details
- **Redump.org** - Xbox 360 game database
- **Archive.org** - Xbox 360 preservation archives

## Compatibility Status Levels

- **Unknown** (0): Game not tested yet
- **Broken** (1): Doesn't boot or completely unplayable
- **Loads** (2): Gets to menu but crashes frequently
- **Gameplay** (3): Playable with noticeable issues
- **Playable** (4): Works well with minor glitches
- **Perfect** (5): No issues detected

## Known Issue Types

The system tracks these common problems:
- Graphics corruption
- Audio glitches
- Memory leaks
- Frequent crashes
- Slow performance
- Input issues
- Save game problems
- Networking issues
- Cutscene problems
- Physics glitches
- Texture/Shader issues

## Community Contributions

### Sharing Your Data

Your crash recovery database is saved to:
```
<xenia_root>/crash_recovery.db
```

This file contains:
- Crash locations and frequencies
- Applied workarounds
- Blacklisted addresses

You can share this data to help improve Xenia!

### Future Enhancements

Planned features:
- **Online database updates**: Download latest compatibility data
- **Automatic issue reporting**: Submit crash data to improve Xenia
- **Community voting**: Rate game compatibility
- **Fix verification**: Test if fixes actually help

## Technical Details

### Architecture

```
┌─────────────────────────────────────┐
│   Emulator Launch                    │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  Game Compatibility Database         │
│  - Initialize built-in fixes         │
│  - Load community updates            │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  Game Loads (Title ID detected)     │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  Apply Game-Specific Fixes           │
│  - Memory configuration              │
│  - Graphics settings                 │
│  - CPU workarounds                   │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  Install Crash Handlers              │
│  - Exception monitoring              │
│  - Automatic recovery                │
│  - Learning system                   │
└─────────────────────────────────────┘
```

### Files

- **crash_recovery.h/cc**: Crash detection and recovery system
- **game_compatibility.h/cc**: Game database and fix application
- **emulator.cc**: Integration with emulator core
- **xenia_main.cc**: Initialization on startup

### API Usage

```cpp
// Check if a game has known issues
auto& db = GameCompatibilityDatabase::GetInstance();
if (db.HasGameInfo(title_id)) {
  auto info = db.GetGameInfo(title_id);
  // Apply fixes
  db.ApplyFixes(title_id);
}

// Record a crash
CrashInfo crash;
crash.type = CrashType::MemoryAccess;
crash.address = 0x82000000;
CrashRecoveryManager::GetInstance().RecordCrash(crash);

// Check if an address is problematic
if (CrashRecoveryManager::GetInstance().IsProblematicAddress(addr)) {
  // Skip or use workaround
}
```

## Resources

### Game Information Sources
- **xenia-project/game-compatibility** - Official compatibility list
- **Archive.org** - Xbox 360 game preservation
- **Redump.org** - Verified game dumps
- **Xbox Forums** - Community knowledge
- **GitHub Issues** - Known emulation problems

### Contributing

To contribute new game fixes:
1. Test the game thoroughly
2. Note any crashes or issues
3. Create fixes in game_compatibility.cc
4. Test that fixes work
5. Submit a pull request

## FAQ

**Q: Will this slow down the emulator?**
A: No! Fixes are applied once at game load. Runtime overhead is minimal.

**Q: Can I disable the compatibility system?**
A: Not currently, but you can disable specific fixes by editing the code.

**Q: My game crashes but there's no fix?**
A: Check crash_recovery.db - the system may be learning. After 3 crashes, it auto-applies a workaround.

**Q: How accurate are the compatibility ratings?**
A: Ratings are conservative estimates. Your experience may vary based on your hardware.

**Q: Can I add my own fixes?**
A: Yes! Edit game_compatibility.cc and rebuild Xenia.

## Troubleshooting

### Game Still Crashes
1. Check the log for compatibility information
2. Verify the Title ID is correct
3. Look for crash recovery messages
4. Share your crash_recovery.db on GitHub Issues

### Fix Not Applied
1. Ensure the game's Title ID matches the database
2. Check that the fix is enabled (`fix.enabled = true`)
3. Verify the fix priority is high enough
4. Look for application messages in the log

### Performance Issues
1. Check if memory configuration is too large
2. Disable graphics workarounds if not needed
3. Try different compatibility settings

## Future Development

The compatibility system is designed to evolve:
- Machine learning for automatic fix generation
- Crowdsourced compatibility database
- Per-game performance profiles
- Automatic game patching
- Save state compatibility tracking

---

**Note**: This system is constantly improving. Check for updates regularly and contribute your findings to help all Xenia users!
