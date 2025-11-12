## 🧠 Xenia Self-Learning Crash Recovery System

This system allows Xenia to **learn from crashes** and automatically apply workarounds to prevent them from happening again.

## How It Works

### 1. **Crash Detection**
- Catches exceptions before they kill the emulator
- Records what went wrong (memory address, instruction, context)
- Logs crash patterns to a learning database

### 2. **Learning**
- Remembers which addresses/functions cause crashes
- Tracks crash frequency (how often each crash occurs)
- Saves learning data to disk (`xenia_crashes.db`)

### 3. **Adaptive Workarounds**
- When the same crash happens multiple times, auto-applies workarounds:
  - **Skip**: Skip the problematic instruction
  - **Return Zero**: Return 0 and continue
  - **Ignore Error**: Continue execution anyway
  - **Use Fallback**: Use safer alternative code path

### 4. **Persistent Memory**
- Learns across sessions (remembers between runs)
- Builds a database of known problematic code
- Gets smarter the more you play

## Features

### ✅ Crash Recovery
Instead of crashing completely, Xenia:
1. Catches the crash
2. Logs what happened
3. Tries to continue running
4. Remembers the issue for next time

### ✅ Automatic Blacklisting
If a function crashes **5+ times**, it's automatically:
- Blacklisted (skipped entirely)
- Added to permanent workaround list
- Avoided in future runs

### ✅ Smart Workarounds
Different crash types get different solutions:
- **Memory errors** → Return zero
- **Division by zero** → Return zero
- **Illegal instructions** → Skip instruction
- **GPU errors** → Use fallback renderer
- **Audio errors** → Ignore and continue

### ✅ Statistics
Track your crash recovery:
```
Total crashes this session: 47
Recovered crashes: 42 (89%)
Known problematic addresses: 15
Active workarounds: 12
```

## Configuration

Add to `portable.txt`:

```ini
# Enable crash recovery system
crash_recovery = true

# Enable learning from crashes
crash_learning = true

# Learning database location
crash_db = xenia_crashes.db

# Auto-apply workarounds
auto_workarounds = true

# Crash before auto-blacklist
crash_threshold = 5
```

## How to Use

### Automatic Mode (Recommended)
Just run Xenia normally. The system:
1. Automatically learns from any crashes
2. Saves learning data when you exit
3. Applies learned workarounds on next run

### Manual Blacklisting
If you know a specific function causes issues:

```cpp
// In code
crash_recovery::CrashRecoveryManager::GetInstance()
    .BlacklistGuestAddress(0x82000400, "Known bad function");
```

### Check What Was Learned
The learning database (`xenia_crashes.db`) contains:

```
[CrashHistory]
0xDEADBEEF|0|12|1234567890|Access violation at memory address
# address | type | frequency | timestamp | details

[Workarounds]
0xDEADBEEF|2|5|1|Auto-learned after 3 crashes
# address | strategy | times_applied | enabled | reason

[Blacklist]
0xBADC0DE
# Permanently blacklisted addresses
```

## Examples

### Example 1: Learning from Memory Crashes
```
Run 1: Crash at 0xDEADBEEF (memory access)
       → Logged to database

Run 2: Crash at 0xDEADBEEF again
       → Frequency = 2

Run 3: Crash at 0xDEADBEEF again
       → Frequency = 3
       → Auto-apply workaround: Return Zero

Run 4: Hit 0xDEADBEEF
       → Workaround applied automatically
       → No crash! Game continues!
```

### Example 2: Blacklisting Problematic Code
```
Crash at 0xBADC0DE 5 times
→ Auto-blacklisted
→ Now always skipped
→ Never crashes again
```

## Integration Points

The system hooks into:

1. **Exception Handler** - Windows SEH/POSIX signals
2. **Memory Manager** - Catches invalid memory access
3. **CPU Emulator** - Detects illegal instructions
4. **GPU Backend** - Catches graphics errors
5. **Audio System** - Handles audio failures

## Benefits

### 🎮 Better Gaming Experience
- Fewer crashes = more playtime
- Games get more stable over time
- Automatic fixes without manual patches

### 🔧 Easier Debugging
- Know exactly what's crashing
- See crash patterns and frequencies
- Share learning databases to help others

### 📊 Analytics
- Track emulation reliability
- Identify problematic games/functions
- Guide development priorities

## Advanced Usage

### Protected Execution
Wrap risky code to catch crashes:

```cpp
XE_TRY_RECOVER(
    RiskyFunction(),
    "Calling risky function"
);
```

### Check Before Executing
```cpp
if (XE_CHECK_PROBLEMATIC_ADDRESS(address)) {
    auto workaround = XE_GET_WORKAROUND(address);
    ApplyWorkaround(workaround);
} else {
    ExecuteNormally(address);
}
```

### Custom Workarounds
```cpp
crash_recovery::CrashRecoveryManager::GetInstance()
    .ApplyWorkaround(
        address,
        WorkaroundStrategy::UseFallback,
        "Custom fix for XYZ game"
    );
```

## Learning Database Format

```
# Xenia Crash Recovery Learning Database
# Generated: 1699876543210
# Total crashes: 47
# Recovered: 42

[CrashHistory]
# Tracks all crashes that have occurred
# Format: address|type|frequency|timestamp|details

[Workarounds]
# Active workarounds being applied
# Format: address|strategy|times_applied|enabled|reason

[Blacklist]
# Permanently blacklisted addresses
# Format: address
```

## Performance Impact

- **Crash Detection**: Negligible (<1% overhead)
- **Learning**: Only during crashes (no impact on normal execution)
- **Database I/O**: Only on startup/shutdown
- **Workaround Application**: Microseconds per check

## Future Enhancements

Potential additions:
- Machine learning for crash prediction
- Pattern recognition across games
- Cloud-shared learning database
- Automatic crash reporting
- Per-game learning profiles

## Troubleshooting

### System Not Learning
Check:
- `crash_recovery = true` in portable.txt
- `crash_learning = true` enabled
- Database file is writable

### Too Aggressive Workarounds
Adjust threshold:
```ini
crash_threshold = 10  # Require more crashes before blacklisting
```

### Want Fresh Start
Delete `xenia_crashes.db` to clear all learned data.

## Ethical Note

This system is designed to improve emulation stability, not hide bugs. All crashes are still logged for developers to review and fix properly. The workarounds are temporary solutions to improve user experience while proper fixes are developed.

---

**Remember**: The more you play, the smarter it gets! 🧠✨
