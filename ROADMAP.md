# Xenia-XT Development Roadmap

## Executive Summary

This roadmap outlines the path to making Xenia-XT an enterprise-grade Xbox 360 emulator. Based on analysis of:
- [Xenia Canary Issues](https://github.com/xenia-canary/xenia-canary/issues)
- [Xenia Knowledge Base](https://xenia-emulator.com/knowledge-base/)
- Internal codebase analysis

**Current State:** 75.3% kernel implementation (301 implemented, 106 stubbed)

---

## Phase 1: Critical Infrastructure (Weeks 1-4)

### 1.1 Graphics Initialization (CRITICAL BLOCKER)
**Status:** 36% complete (8/22 functions)
**Impact:** Games cannot render without these

| Function | Priority | Complexity | Description |
|----------|----------|------------|-------------|
| `VdInitializeEngines` | P0 | Hard | GPU initialization - MUST HAVE |
| `VdSetDisplayMode` | P0 | Medium | Display mode setup |
| `VdQueryVideoMode` | P0 | Easy | Query current video mode |
| `VdGetCurrentDisplayInformation` | P1 | Easy | Display properties |
| `VdSetDisplayModeOverride` | P1 | Medium | Override display settings |

**Files:** `src/xenia/kernel/xboxkrnl/xboxkrnl_video.cc`

### 1.2 File I/O Completion (HIGH PRIORITY)
**Status:** 50% complete (13/26 functions)
**Impact:** Game saves and asset loading failures

| Function | Priority | Complexity | Description |
|----------|----------|------------|-------------|
| `NtDeviceIoControlFile` | P0 | Hard | Device I/O control |
| `IoCompleteRequest` | P0 | Medium | Async I/O completion |
| `StfsCreateDevice` | P0 | Hard | Save file system mounting |
| `IoCreateDevice` | P1 | Medium | Generic device creation |
| `IoDeleteDevice` | P1 | Easy | Device cleanup |

**Files:** `src/xenia/kernel/xboxkrnl/xboxkrnl_io.cc`

### 1.3 Audio Quality Improvements
**Status:** Frequent complaints about compressed/low-quality audio
**Impact:** User experience, immersion

**Issues to Address:**
- [ ] Achievement notification audio missing
- [ ] Audio popping and stuttering
- [ ] XMA decoder quality improvements
- [ ] Audio timing synchronization

**Files:** `src/xenia/apu/`

---

## Phase 2: Kernel Completion (Weeks 5-8)

### 2.1 Networking Layer
**Status:** 55% complete (11 stubs in XAM)
**Impact:** Online/multiplayer games non-functional

| Function | Priority | Complexity |
|----------|----------|------------|
| `NetDll_XNetXnAddrToInAddr` | P1 | Easy |
| `NetDll_XNetDnsLookup` | P1 | Medium |
| `NetDll_WSARecvFrom` | P1 | Medium |
| `NetDll_XNetConnect` | P2 | Medium |
| `NetDll_getsockopt` | P2 | Easy |

**Files:** `src/xenia/kernel/xam/xam_net.cc`

### 2.2 User/Account System
**Status:** 66% complete (13 stubs)
**Impact:** Parental controls, friends, achievements

| Function | Priority | Complexity |
|----------|----------|------------|
| `XamUserCheckPrivilege` | P1 | Easy |
| `XamPartyGetUserList` | P1 | Medium |
| `XamShowPartyUI` | P2 | Medium |
| `XamUserContentRestrictionCheckAccess` | P2 | Easy |

**Files:** `src/xenia/kernel/xam/xam_user.cc`, `src/xenia/kernel/xam/xam_party.cc`

### 2.3 Exception Handling (SEH)
**Status:** Partial - causes crashes in some games
**Impact:** Soul Calibur V and similar titles

**Tasks:**
- [ ] Implement proper `RtlUnwind` for stack unwinding
- [ ] Fix `RtlRaiseException` continuation for non-fatal exceptions
- [ ] Implement `__C_specific_handler` properly
- [ ] Add exception frame tracking

**Files:** `src/xenia/kernel/xboxkrnl/xboxkrnl_debug.cc`, `src/xenia/kernel/xboxkrnl/xboxkrnl_rtl.cc`

---

## Phase 3: GPU Accuracy (Weeks 9-16)

### 3.1 eDRAM Emulation Improvements
**Status:** Working but incomplete
**Impact:** Rendering accuracy, transparency issues

Based on [Xenia GPU documentation](https://xenia-emulator.com/knowledge-base/gpu-emulation/):

**Tasks:**
- [ ] More accurate eDRAM behavior and blending models
- [ ] Fix resolve region empty warnings (300+ per session)
- [ ] Improve MSAA handling
- [ ] Better depth/stencil buffer management

**Files:** `src/xenia/gpu/d3d12/d3d12_render_target_cache.cc`

### 3.2 Shader Translation
**Status:** Functional but causes stuttering
**Impact:** Performance, visual glitches

**Tasks:**
- [ ] Better shader precompilation and caching
- [ ] Fix texture flickering issues
- [ ] Improve shader cache persistence
- [ ] Add missing shader operations

**Files:** `src/xenia/gpu/dxbc_shader_translator.cc`, `src/xenia/gpu/spirv_shader_translator.cc`

### 3.3 Vulkan Backend Improvements
**Status:** Under active development
**Impact:** Linux support, AMD GPU compatibility

**Tasks:**
- [ ] Complete Vulkan feature parity with D3D12
- [ ] Fix display tearing issues
- [ ] Improve Vulkan shader compilation
- [ ] Better memory management on Vulkan

**Files:** `src/xenia/gpu/vulkan/`

---

## Phase 4: Performance & Stability (Weeks 17-20)

### 4.1 CPU Emulation Optimization
**Status:** Functional, some edge cases

**Tasks:**
- [ ] Fix "sketchy" implementations (8 functions identified)
- [ ] Improve JIT code generation
- [ ] Better branch prediction handling
- [ ] Fix sign extension issues in 64-bit registers

**Files:** `src/xenia/cpu/`

### 4.2 Memory Management
**Status:** 85.7% complete
**Impact:** Stability, crash prevention

**Tasks:**
- [ ] Implement `MmQueryStatistics` properly
- [ ] Better L2 cache simulation
- [ ] Fix memory fragmentation issues
- [ ] Improve large page support

**Files:** `src/xenia/kernel/xboxkrnl/xboxkrnl_memory.cc`

### 4.3 Threading & Synchronization
**Status:** 94.3% complete (66/70 functions)
**Impact:** Race conditions, deadlocks

**Tasks:**
- [ ] Fix `KeInsertQueueDpc` (marked sketchy)
- [ ] Implement proper deadlock detection
- [ ] Better thread priority handling
- [ ] Fix DPC queue issues

**Files:** `src/xenia/kernel/xboxkrnl/xboxkrnl_threading.cc`

---

## Phase 5: Game Compatibility Database (Ongoing)

### 5.1 Compatibility Testing Framework
**Tasks:**
- [ ] Automated game boot testing
- [ ] Performance regression detection
- [ ] Screenshot comparison for visual accuracy
- [ ] Crash log aggregation and analysis

### 5.2 Per-Game Fixes System
**Current:** 9 games with fixes in `game_compatibility.cc`

**Expand to include:**
- Memory configuration overrides
- CPU workarounds for specific addresses
- Graphics setting tweaks
- Audio configuration

### 5.3 Community Database Integration
**Tasks:**
- [ ] Implement `UpdateFromURL()` for remote database
- [ ] JSON/TOML parser for compatibility data
- [ ] User-submitted fix system
- [ ] Automatic fix application on game launch

---

## Priority Matrix

### Tier 1 - Critical (Implement First)
1. `VdInitializeEngines` - GPU setup
2. `VdSetDisplayMode` / `VdQueryVideoMode` - Display control
3. `NtDeviceIoControlFile` - Device I/O
4. `IoCompleteRequest` - Async I/O callbacks
5. `StfsCreateDevice` - Save file system

**Expected Impact:** 40-50% improvement in game compatibility

### Tier 2 - High Priority
1. Network address conversion (5 functions)
2. User privilege functions
3. `KeInsertQueueDpc` (fix sketchy implementation)
4. Audio quality improvements
5. Exception handling fixes

**Expected Impact:** Additional 30% improvement

### Tier 3 - Medium Priority
1. DNS lookup functions
2. Display info queries
3. Cryptography improvements
4. Vulkan parity
5. Shader caching improvements

**Expected Impact:** Additional 15% improvement

---

## Known Broken Games & Root Causes

| Game | Title ID | Root Cause | Fix Complexity |
|------|----------|------------|----------------|
| Soul Calibur V | 4E4D083D | XamParty functions | Medium |
| Red Dead Redemption | 5454082B | Memory/streaming | Has workarounds |
| GTA V | 5454087C | Memory/texture streaming | Has workarounds |
| Halo: Reach | 4D530919 | GPU features | Hard |
| Skyrim | 425307D6 | Memory fragmentation | Has workarounds |

---

## Technical Debt

### Code Quality Issues
- [ ] 50+ TODO/FIXME comments throughout codebase
- [ ] 8 implementations marked as "sketchy"
- [ ] Inconsistent error handling patterns
- [ ] Missing unit tests for kernel functions

### Documentation Gaps
- [ ] Undocumented system calls need reverse engineering
- [ ] GPU register documentation incomplete
- [ ] Threading model documentation needed

---

## Success Metrics

### Short Term (3 months)
- [ ] 80% of games boot to menu
- [ ] 60% of games playable
- [ ] Audio quality complaints reduced by 50%
- [ ] No critical kernel crashes

### Medium Term (6 months)
- [ ] 90% of games boot to menu
- [ ] 75% of games playable
- [ ] Vulkan feature parity with D3D12
- [ ] Automated compatibility testing

### Long Term (12 months)
- [ ] 95% of games boot
- [ ] 85% of games playable
- [ ] Enterprise-grade stability
- [ ] Community contribution pipeline

---

## Resources

- [Xenia Official](https://xenia.jp/)
- [Xenia Canary Issues](https://github.com/xenia-canary/xenia-canary/issues)
- [GPU Emulation Deep Dive](https://xenia.jp/updates/2021/04/27/leaving-no-pixel-behind-new-render-target-cache-3x3-resolution-scaling.html)
- [Kernel API Documentation](https://xenia-emulator.com/knowledge-base/kernel-system-api-emulation/)
- [Xbox 360 Development Wiki](https://free60.org/)

---

## Next Steps

1. **Immediate:** Start with `VdInitializeEngines` implementation
2. **This Week:** Audit all 14 video stubs for implementation feasibility
3. **This Month:** Complete Phase 1 critical infrastructure
4. **Review:** Weekly progress check against this roadmap

---

*Last Updated: November 24, 2025*
*Version: 1.0*
