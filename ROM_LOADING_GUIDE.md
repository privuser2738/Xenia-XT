# Xbox 360 ROM Loading - Comprehensive Guide

## Problem Solved

The ISO file `c:/Games/sfxt.iso` was **truncated** - only 606 MB of data when it should have been ~1.2 GB. This caused loading failures.

## Solution Implemented

Added **graceful error handling** that:
1. ✅ Detects out-of-bounds directories/files
2. ✅ Warns about inaccessible content
3. ✅ Continues loading accessible content
4. ✅ Provides detailed summary of what loaded
5. ✅ Successfully mounts partial ISOs

## Results for sfxt.iso

### Successfully Loaded (8 entries):
- **Files (5)**:
  - `/$SystemUpdate/su20076000_00000000`
  - `/$SystemUpdate/system.manifest`
  - `/AvatarAssetPack`
  - `/default.xex` ← **Main executable accessible!**
  - `/nxeart`
- **Directories (3)**:
  - `/$SystemUpdate`
  - Root entries

### Inaccessible (Truncated):
- `/archive` directory (sector 483515 = 0x4ADED800, **591 MB beyond file end**)
- `/stream` directory (sector 1551408 = 0xCD3A8000, **beyond file end**)

## Running ROMs with Verbose Logging

To test any ROM and capture detailed logs:

```bash
cd c:/Users/rob/source/xenia
./build/bin/Windows/Debug/xenia.exe "path/to/game.iso" > "logs/game_name.log" 2>&1
```

## Log Output Includes:

1. **File loading details**
   - GDFX magic location
   - Root directory info
   - Entry-by-entry parsing

2. **Warnings for issues**
   - Out-of-bounds directories
   - Inaccessible files
   - Corrupt entries

3. **Load summary**
   - Total entries loaded
   - Files vs directories count
   - Complete list of accessible files

4. **Launch attempt**
   - Which executable was found
   - File resolution attempts

## For Future ROM Testing:

### Test Script:
```bash
#!/bin/bash
# Test multiple ROMs and log results

ROMS_DIR="c:/Games"
LOGS_DIR="c:/Users/rob/source/xenia/rom_logs"
XENIA="c:/Users/rob/source/xenia/build/bin/Windows/Debug/xenia.exe"

mkdir -p "$LOGS_DIR"

for iso in "$ROMS_DIR"/*.iso; do
    name=$(basename "$iso" .iso)
    echo "Testing: $name"
    timeout 30 "$XENIA" "$iso" > "$LOGS_DIR/${name}_load.log" 2>&1
    echo "  Log: $LOGS_DIR/${name}_load.log"
done

# Generate summary
echo "=== ROM Loading Summary ===" > "$LOGS_DIR/SUMMARY.txt"
for log in "$LOGS_DIR"/*_load.log; do
    name=$(basename "$log" _load.log)
    echo "" >> "$LOGS_DIR/SUMMARY.txt"
    echo "ROM: $name" >> "$LOGS_DIR/SUMMARY.txt"
    grep "Total entries loaded\|Files:\|Directories:\|SUCCESS\|FAILED" "$log" >> "$LOGS_DIR/SUMMARY.txt"
done
```

## Key Log Indicators:

### Success Indicators:
- `✓ GDFX magic found at game_offset`
- `✓ GDFX header validated successfully`
- `SUCCESS: Loaded X accessible entries`
- `=== Disc Image Loaded Successfully ===`

### Warning Indicators:
- `WARNING: Directory '...' sector ... exceeds file size`
- `WARNING: File '...' sector ... exceeds file size`
- `This directory is INACCESSIBLE (truncated/corrupt ISO)`

### Failure Indicators:
- `GDFX magic not found - this is not a valid Xbox 360 disc image`
- `Root offset ... exceeds file size`
- `FAILED: No entries could be loaded`

## Troubleshooting:

### ISO won't load at all:
- **Not a valid Xbox 360 disc**: Download correct format
- **Completely corrupt**: Re-rip from disc

### Partial loading (like sfxt.iso):
- **Truncated download**: Re-download complete ISO
- **Transfer error**: Copy again from source
- **Works but limited**: Some content may be playable

### Game crashes after loading:
- **Missing game files**: They're in truncated sections
- **Need complete ISO**: Get full version

## File Format Notes:

### Valid Xbox 360 formats:
- `.iso` - GDFX/XISO disc images
- `.xex` - Standalone executables (XBLA games)
- STFS containers - Xbox Live Arcade packages

### GDFX Structure:
- Magic: "MICROSOFT*XBOX*MEDIA" at sector 32
- Root directory referenced by sector number
- Binary tree structure for fast lookups
- Variable-sized entries (4-byte aligned)

## Next Steps:

1. **For sfxt.iso**: Get complete 1.2+ GB version
2. **Test other ROMs**: Use script above to batch test
3. **Review logs**: Check `rom_logs/SUMMARY.txt`
4. **Report issues**: Logs contain full diagnostic info

## Changes Made to Code:

**File**: `src/xenia/vfs/devices/disc_image_device.cc`

**Key modifications**:
1. Added robust error handling for out-of-bounds reads
2. Changed errors to warnings to continue loading
3. Added comprehensive logging at all stages
4. Generate summary of loaded content
5. List all accessible files

This ensures maximum compatibility with partial/damaged ISOs while providing detailed diagnostics for troubleshooting.
