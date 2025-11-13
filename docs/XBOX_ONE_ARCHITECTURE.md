# Xbox One/Series X|S Architecture Reference

## Overview

This document provides reference information on Xbox One and Series X|S architecture for comparison with Xbox 360 (which Xenia emulates). **Note: Xenia does not and cannot emulate Xbox One/Series consoles.**

## Why Xbox One/Series Cannot Be Added to Xenia

### Fundamental Architecture Differences

| Component | Xbox 360 (Xenia) | Xbox One/Series X|S |
|-----------|------------------|---------------------|
| **CPU** | PowerPC Xenon (3-core @ 3.2 GHz) | x86-64 AMD Jaguar (8-core @ 1.75 GHz) / Zen 2 (8-core @ 3.6-3.8 GHz) |
| **GPU** | AMD Xenos (R600-based, 240 GFLOPS) | AMD GCN (1.31 TFLOPS) / RDNA 2 (12-20 TFLOPS) |
| **Memory** | 512 MB GDDR3 unified | 8-16 GB DDR3/GDDR6 |
| **OS** | Xbox 360 Kernel (custom) | Windows 10/11-based Hyper-V |
| **Format** | XEX/XEX2 | UWP (Universal Windows Platform) |
| **API** | DirectX 9-era | DirectX 11/12 |

### Technical Barriers

1. **Completely Different CPU Architecture**
   - Xbox 360: Big-endian PowerPC RISC
   - Xbox One: Little-endian x86-64 CISC
   - Would require entirely new CPU emulator

2. **Different GPU Programming Model**
   - Xbox 360: Command buffer + tiles
   - Xbox One: Unified memory architecture + compute shaders
   - Completely different GPU backends required

3. **Operating System Layer**
   - Xbox 360: Custom lightweight kernel
   - Xbox One: Full Windows kernel with hypervisor
   - Would need to emulate Windows NT kernel

4. **Security & DRM**
   - Xbox One games use modern encryption
   - Keys not publicly available
   - Active commercial platform

5. **Executable Format**
   - Xbox 360: XEX format (portable executable)
   - Xbox One: UWP packages with AppX containers
   - Different loading, linking, and security model

## Xbox 360 Backwards Compatibility on Xbox One

### How It Works

Xbox One runs Xbox 360 games through **native emulation**, not recompilation:

1. **Hypervisor-Based Emulation**
   - Xbox One uses a Hyper-V hypervisor
   - Runs a custom Xbox 360 virtual machine
   - Emulates PowerPC CPU in software
   - Translates Xenos GPU commands to GCN

2. **Per-Game Packages**
   - Each BC title gets a custom wrapper
   - Microsoft hand-tunes emulation per game
   - Includes game-specific compatibility fixes
   - Distributed as special packages via Xbox Live

3. **Enhancements**
   - Resolution upscaling (2x-4x)
   - Improved texture filtering
   - Forced 16x anisotropic filtering
   - Higher framerates (when possible)
   - HDR injection (select titles)

4. **Limitations**
   - Not all games are compatible
   - Requires Microsoft's approval
   - Original disc + download required
   - No homebrew/unsigned code

### BC Detection in Xenia

Xenia cannot detect if a game is "BC-enhanced" because:
- Enhancements are applied by Xbox One's emulator
- Original XEX files are unmodified
- BC packages contain wrapper code, not XEX changes

However, games with good BC compatibility often have:
- Standard system flags
- Proper resource management
- Well-behaved memory access
- Limited hardware-specific hacks

## Xbox Series X|S Enhancements

Series X|S adds additional BC features:

- **FPS Boost**: Doubles or quadruples frame rates (30→60, 60→120)
- **Auto HDR**: ML-based HDR injection for SDR games
- **Quick Resume**: Suspend/resume multiple games
- **Storage**: Games run from NVMe SSD

## Implications for Xenia Development

### What We Can Learn

1. **Compatibility Patterns**: Games that run well on Xbox One BC often:
   - Use standard APIs properly
   - Have predictable memory patterns
   - Don't rely on hardware quirks

2. **Enhancement Techniques**: Xbox One BC shows what's possible:
   - Resolution scaling without game changes
   - Texture filtering improvements
   - Frame timing fixes

### What We Cannot Do

1. **Emulate Xbox One**: Architecture too different
2. **Run Xbox One Games**: Requires different emulator
3. **Add BC Enhancements**: Would need per-game profiles

## Future Xbox Emulation

For Xbox One/Series emulation:

1. **Separate Project Needed**: Not compatible with Xenia
2. **Legal Concerns**: Active commercial platform
3. **Technical Difficulty**: x86-64 emulation on x86-64 requires:
   - Ring 0 execution for kernel emulation
   - Hypervisor-level access
   - GPU command stream translation

4. **Existing Efforts**: No mature Xbox One emulators exist as of 2025

## References

- [Xbox 360 System Architecture](https://www.anandtech.com/show/1689)
- [Xbox One Hardware Analysis](https://www.eurogamer.net/digitalfoundry-2020-inside-xbox-series-x-full-specs)
- [Backwards Compatibility on Xbox](https://news.xbox.com/en-us/2017/10/24/xbox-one-backward-compatibility/)
- [XEX Format Documentation](https://free60.org/wiki/XEX)

## Contributing

If you're interested in Xbox One emulation:
- This is not the right project (Xenia is Xbox 360 only)
- Start a new project focused on x86-64 emulation
- Study existing PC emulators (RPCS3 for PS3 is similar architecture)

---

**Last Updated**: 2025-11-12
**Xenia Version**: master@1d04cdcf9
