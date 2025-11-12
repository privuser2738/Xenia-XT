/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_BASE_CRASH_RECOVERY_H_
#define XENIA_BASE_CRASH_RECOVERY_H_

#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace xe {
namespace crash_recovery {

// Crash types we can learn from
enum class CrashType {
  MemoryAccess,      // Invalid memory access
  DivideByZero,      // Division by zero
  IllegalInstruction, // Invalid CPU instruction
  StackOverflow,     // Stack overflow
  GPUError,          // GPU-related crash
  AudioError,        // Audio-related crash
  Unknown,           // Unknown crash type
};

// Information about a crash
struct CrashInfo {
  CrashType type;
  uint64_t address;          // Memory address or instruction pointer
  uint32_t guest_address;    // Xbox 360 guest address if available
  std::string function_name; // Function name if available
  std::string details;       // Additional details
  uint32_t frequency;        // How many times this has crashed
  uint64_t timestamp;        // When it crashed
};

// Workaround strategies
enum class WorkaroundStrategy {
  Skip,              // Skip the problematic code
  ReturnZero,        // Return 0 and continue
  ReturnSuccess,     // Return success status
  UseFallback,       // Use fallback implementation
  DisableFeature,    // Disable the feature causing crashes
  IgnoreError,       // Ignore the error and continue
};

// A learned workaround
struct Workaround {
  uint64_t address;
  WorkaroundStrategy strategy;
  std::string reason;
  uint32_t times_applied;
  bool enabled;
};

// Main crash recovery manager
class CrashRecoveryManager {
 public:
  static CrashRecoveryManager& GetInstance();

  // Initialize the system
  void Initialize(const std::string& learning_db_path);
  void Shutdown();

  // Install crash handlers
  void InstallCrashHandlers();

  // Record a crash
  void RecordCrash(const CrashInfo& crash);

  // Check if an address is known to be problematic
  bool IsProblematicAddress(uint64_t address) const;
  bool IsProblematicGuestAddress(uint32_t guest_address) const;

  // Get recommended workaround for an address
  WorkaroundStrategy GetWorkaround(uint64_t address) const;
  WorkaroundStrategy GetGuestWorkaround(uint32_t guest_address) const;

  // Apply a workaround
  void ApplyWorkaround(uint64_t address, WorkaroundStrategy strategy,
                       const std::string& reason);

  // Blacklist an address (always skip/avoid)
  void BlacklistAddress(uint64_t address, const std::string& reason);
  void BlacklistGuestAddress(uint32_t guest_address, const std::string& reason);

  // Get crash statistics
  uint32_t GetTotalCrashes() const { return total_crashes_; }
  uint32_t GetRecoveredCrashes() const { return recovered_crashes_; }
  std::vector<CrashInfo> GetRecentCrashes(size_t count = 10) const;
  std::vector<CrashInfo> GetFrequentCrashes(size_t count = 10) const;

  // Save/load learning database
  void SaveLearningDatabase();
  void LoadLearningDatabase();

  // Execute code with crash protection
  template <typename Func>
  bool TryExecute(Func&& func, const std::string& context) {
    try {
      func();
      return true;
    } catch (...) {
      RecordCrash({CrashType::Unknown, 0, 0, context, "Exception caught", 1,
                   GetCurrentTimestamp()});
      return false;
    }
  }

  // Enable/disable learning
  void SetLearningEnabled(bool enabled) { learning_enabled_ = enabled; }
  bool IsLearningEnabled() const { return learning_enabled_; }

  // Enable/disable workarounds
  void SetWorkaroundsEnabled(bool enabled) { workarounds_enabled_ = enabled; }
  bool AreWorkaroundsEnabled() const { return workarounds_enabled_; }

 private:
  CrashRecoveryManager() = default;
  ~CrashRecoveryManager() = default;

  // No copy/move
  CrashRecoveryManager(const CrashRecoveryManager&) = delete;
  CrashRecoveryManager& operator=(const CrashRecoveryManager&) = delete;

  uint64_t GetCurrentTimestamp() const;
  void AnalyzeCrashPatterns();
  WorkaroundStrategy DetermineWorkaround(const CrashInfo& crash) const;

  std::string learning_db_path_;
  std::map<uint64_t, CrashInfo> crash_history_;      // Address -> crash info
  std::map<uint32_t, CrashInfo> guest_crash_history_; // Guest addr -> crash info
  std::map<uint64_t, Workaround> workarounds_;
  std::set<uint64_t> blacklisted_addresses_;
  std::set<uint32_t> blacklisted_guest_addresses_;

  uint32_t total_crashes_ = 0;
  uint32_t recovered_crashes_ = 0;
  bool learning_enabled_ = true;
  bool workarounds_enabled_ = true;
  bool initialized_ = false;
};

// Helper macros for protected execution
#define XE_TRY_RECOVER(code, context)                                     \
  xe::crash_recovery::CrashRecoveryManager::GetInstance().TryExecute(     \
      [&]() { code; }, context)

#define XE_CHECK_PROBLEMATIC_ADDRESS(addr)                                \
  xe::crash_recovery::CrashRecoveryManager::GetInstance()                 \
      .IsProblematicAddress(addr)

#define XE_GET_WORKAROUND(addr)                                           \
  xe::crash_recovery::CrashRecoveryManager::GetInstance().GetWorkaround(addr)

}  // namespace crash_recovery
}  // namespace xe

#endif  // XENIA_BASE_CRASH_RECOVERY_H_
