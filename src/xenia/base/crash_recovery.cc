/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/crash_recovery.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/logging.h"

#if XE_PLATFORM_WIN32
#include <Windows.h>
#include <signal.h>
#endif

namespace xe {
namespace crash_recovery {

namespace {

// Global crash handler
CrashRecoveryManager* g_crash_manager = nullptr;

#if XE_PLATFORM_WIN32
// Windows exception handler
LONG WINAPI CrashExceptionHandler(EXCEPTION_POINTERS* ex_info) {
  if (!g_crash_manager) {
    return EXCEPTION_CONTINUE_SEARCH;
  }

  CrashInfo crash;
  crash.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  crash.frequency = 1;

  switch (ex_info->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
      crash.type = CrashType::MemoryAccess;
      crash.address = ex_info->ExceptionRecord->ExceptionAddress
                          ? reinterpret_cast<uint64_t>(
                                ex_info->ExceptionRecord->ExceptionAddress)
                          : 0;
      {
        const char* access_type = "unknown";
        uint64_t fault_address = 0;
        if (ex_info->ExceptionRecord->NumberParameters >= 2) {
          // First parameter: 0 = read, 1 = write, 8 = DEP violation
          switch (ex_info->ExceptionRecord->ExceptionInformation[0]) {
            case 0: access_type = "read"; break;
            case 1: access_type = "write"; break;
            case 8: access_type = "DEP"; break;
          }
          // Second parameter: address that caused the fault
          fault_address = ex_info->ExceptionRecord->ExceptionInformation[1];
        }
        crash.details = fmt::format(
            "Access violation: {} at IP 0x{:X}, accessing memory 0x{:X}",
            access_type, crash.address, fault_address);
        crash.guest_address = static_cast<uint32_t>(fault_address);
      }
      break;

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      crash.type = CrashType::DivideByZero;
      crash.address = reinterpret_cast<uint64_t>(
          ex_info->ExceptionRecord->ExceptionAddress);
      crash.details = "Division by zero";
      break;

    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_PRIV_INSTRUCTION:
      crash.type = CrashType::IllegalInstruction;
      crash.address = reinterpret_cast<uint64_t>(
          ex_info->ExceptionRecord->ExceptionAddress);
      crash.details = "Illegal instruction";
      break;

    case EXCEPTION_STACK_OVERFLOW:
      crash.type = CrashType::StackOverflow;
      crash.address = reinterpret_cast<uint64_t>(
          ex_info->ExceptionRecord->ExceptionAddress);
      crash.details = "Stack overflow";
      break;

    default:
      crash.type = CrashType::Unknown;
      crash.address = reinterpret_cast<uint64_t>(
          ex_info->ExceptionRecord->ExceptionAddress);
      crash.details = fmt::format("Exception code: 0x{:X}",
                                   ex_info->ExceptionRecord->ExceptionCode);
      break;
  }

  // Write crash to console and log file immediately
  XELOGE("!!! CRASH DETECTED !!!");
  XELOGE("Type: {}", static_cast<int>(crash.type));
  XELOGE("Details: {}", crash.details);
  XELOGE("Address: 0x{:X}", crash.address);
  XELOGE("Guest Address: 0x{:X}", crash.guest_address);

  g_crash_manager->RecordCrash(crash);

  // Try to continue execution if we have a workaround
  if (g_crash_manager->AreWorkaroundsEnabled() &&
      g_crash_manager->IsProblematicAddress(crash.address)) {
    auto strategy = g_crash_manager->GetWorkaround(crash.address);

    switch (strategy) {
      case WorkaroundStrategy::Skip:
        // Skip the instruction by advancing IP
#if defined(_M_X64) || defined(__x86_64__)
        ex_info->ContextRecord->Rip += 1;
#endif
        return EXCEPTION_CONTINUE_EXECUTION;

      case WorkaroundStrategy::ReturnZero:
#if defined(_M_X64) || defined(__x86_64__)
        ex_info->ContextRecord->Rax = 0; // Set return value to 0
        ex_info->ContextRecord->Rip += 1;
#endif
        return EXCEPTION_CONTINUE_EXECUTION;

      case WorkaroundStrategy::IgnoreError:
        // Continue execution at next instruction
#if defined(_M_X64) || defined(__x86_64__)
        ex_info->ContextRecord->Rip += 1;
#endif
        return EXCEPTION_CONTINUE_EXECUTION;

      default:
        break;
    }
  }

  // If no workaround available, let it crash but save learning data
  g_crash_manager->SaveLearningDatabase();

  return EXCEPTION_CONTINUE_SEARCH;
}

// Signal handler for POSIX signals
void SignalHandler(int sig) {
  if (!g_crash_manager) {
    return;
  }

  CrashInfo crash;
  crash.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  crash.frequency = 1;
  crash.address = 0;

  switch (sig) {
    case SIGSEGV:
      crash.type = CrashType::MemoryAccess;
      crash.details = "Segmentation fault";
      break;
    case SIGFPE:
      crash.type = CrashType::DivideByZero;
      crash.details = "Floating point exception";
      break;
    case SIGILL:
      crash.type = CrashType::IllegalInstruction;
      crash.details = "Illegal instruction";
      break;
    default:
      crash.type = CrashType::Unknown;
      crash.details = fmt::format("Signal: {}", sig);
      break;
  }

  XELOGI("Signal caught and recorded: {}", crash.details);
  g_crash_manager->RecordCrash(crash);
  g_crash_manager->SaveLearningDatabase();
}
#endif

}  // namespace

CrashRecoveryManager& CrashRecoveryManager::GetInstance() {
  static CrashRecoveryManager instance;
  return instance;
}

void CrashRecoveryManager::Initialize(const std::string& learning_db_path) {
  if (initialized_) {
    return;
  }

  learning_db_path_ = learning_db_path;
  g_crash_manager = this;

  // Load previous learning data
  LoadLearningDatabase();

  XELOGI("Crash Recovery System initialized");
  XELOGI("  Total known crashes: {}", crash_history_.size());
  XELOGI("  Active workarounds: {}", workarounds_.size());
  XELOGI("  Blacklisted addresses: {}", blacklisted_addresses_.size());

  initialized_ = true;
}

void CrashRecoveryManager::Shutdown() {
  if (!initialized_) {
    return;
  }

  // Save what we've learned
  SaveLearningDatabase();

  XELOGI("Crash Recovery System shutdown");
  XELOGI("  Total crashes this session: {}", total_crashes_);
  XELOGI("  Recovered crashes: {}", recovered_crashes_);

  g_crash_manager = nullptr;
  initialized_ = false;
}

void CrashRecoveryManager::InstallCrashHandlers() {
#if XE_PLATFORM_WIN32
  // Install Windows structured exception handler
  SetUnhandledExceptionFilter(CrashExceptionHandler);

  // Install signal handlers
  signal(SIGSEGV, SignalHandler);
  signal(SIGFPE, SignalHandler);
  signal(SIGILL, SignalHandler);

  XELOGI("Crash handlers installed (Windows)");
#else
  // Install POSIX signal handlers
  signal(SIGSEGV, SignalHandler);
  signal(SIGFPE, SignalHandler);
  signal(SIGILL, SignalHandler);

  XELOGI("Crash handlers installed (POSIX)");
#endif
}

void CrashRecoveryManager::RecordCrash(const CrashInfo& crash) {
  total_crashes_++;

  // Update crash history
  auto it = crash_history_.find(crash.address);
  if (it != crash_history_.end()) {
    // Known crash location - increment frequency
    it->second.frequency++;
    it->second.timestamp = crash.timestamp;
    recovered_crashes_++;

    XELOGW("Known crash at 0x{:X} (occurred {} times)", crash.address,
           it->second.frequency);

    // If this crashes frequently, auto-apply workaround
    if (learning_enabled_ && it->second.frequency >= 3 &&
        workarounds_.find(crash.address) == workarounds_.end()) {
      auto strategy = DetermineWorkaround(crash);
      ApplyWorkaround(crash.address, strategy,
                      fmt::format("Auto-learned after {} crashes",
                                  it->second.frequency));
    }
  } else {
    // New crash location
    crash_history_[crash.address] = crash;
    XELOGI("New crash recorded at 0x{:X}: {}", crash.address, crash.details);
  }

  // Periodically analyze patterns
  if (total_crashes_ % 10 == 0) {
    AnalyzeCrashPatterns();
  }
}

bool CrashRecoveryManager::IsProblematicAddress(uint64_t address) const {
  return blacklisted_addresses_.count(address) > 0 ||
         crash_history_.count(address) > 0;
}

bool CrashRecoveryManager::IsProblematicGuestAddress(
    uint32_t guest_address) const {
  return blacklisted_guest_addresses_.count(guest_address) > 0 ||
         guest_crash_history_.count(guest_address) > 0;
}

WorkaroundStrategy CrashRecoveryManager::GetWorkaround(uint64_t address) const {
  auto it = workarounds_.find(address);
  if (it != workarounds_.end() && it->second.enabled) {
    return it->second.strategy;
  }
  return WorkaroundStrategy::IgnoreError;
}

WorkaroundStrategy CrashRecoveryManager::GetGuestWorkaround(
    uint32_t guest_address) const {
  // Check guest crash history
  auto it = guest_crash_history_.find(guest_address);
  if (it != guest_crash_history_.end()) {
    return DetermineWorkaround(it->second);
  }
  return WorkaroundStrategy::IgnoreError;
}

void CrashRecoveryManager::ApplyWorkaround(uint64_t address,
                                            WorkaroundStrategy strategy,
                                            const std::string& reason) {
  Workaround workaround;
  workaround.address = address;
  workaround.strategy = strategy;
  workaround.reason = reason;
  workaround.times_applied = 0;
  workaround.enabled = true;

  workarounds_[address] = workaround;

  XELOGI("Workaround applied at 0x{:X}: {} ({})", address,
         static_cast<int>(strategy), reason);
}

void CrashRecoveryManager::BlacklistAddress(uint64_t address,
                                             const std::string& reason) {
  blacklisted_addresses_.insert(address);
  ApplyWorkaround(address, WorkaroundStrategy::Skip, reason);

  XELOGI("Address blacklisted: 0x{:X} ({})", address, reason);
}

void CrashRecoveryManager::BlacklistGuestAddress(uint32_t guest_address,
                                                  const std::string& reason) {
  blacklisted_guest_addresses_.insert(guest_address);

  XELOGI("Guest address blacklisted: 0x{:X} ({})", guest_address, reason);
}

std::vector<CrashInfo> CrashRecoveryManager::GetRecentCrashes(
    size_t count) const {
  std::vector<CrashInfo> crashes;
  crashes.reserve(crash_history_.size());

  for (const auto& pair : crash_history_) {
    crashes.push_back(pair.second);
  }

  // Sort by timestamp (most recent first)
  std::sort(crashes.begin(), crashes.end(),
            [](const CrashInfo& a, const CrashInfo& b) {
              return a.timestamp > b.timestamp;
            });

  if (crashes.size() > count) {
    crashes.resize(count);
  }

  return crashes;
}

std::vector<CrashInfo> CrashRecoveryManager::GetFrequentCrashes(
    size_t count) const {
  std::vector<CrashInfo> crashes;
  crashes.reserve(crash_history_.size());

  for (const auto& pair : crash_history_) {
    crashes.push_back(pair.second);
  }

  // Sort by frequency (most frequent first)
  std::sort(crashes.begin(), crashes.end(),
            [](const CrashInfo& a, const CrashInfo& b) {
              return a.frequency > b.frequency;
            });

  if (crashes.size() > count) {
    crashes.resize(count);
  }

  return crashes;
}

void CrashRecoveryManager::SaveLearningDatabase() {
  if (learning_db_path_.empty()) {
    return;
  }

  std::ofstream file(learning_db_path_);
  if (!file.is_open()) {
    XELOGW("Failed to save learning database to: {}", learning_db_path_);
    return;
  }

  // Write header
  file << "# Xenia Crash Recovery Learning Database\n";
  file << "# Generated: " << GetCurrentTimestamp() << "\n";
  file << "# Total crashes: " << total_crashes_ << "\n";
  file << "# Recovered: " << recovered_crashes_ << "\n";
  file << "\n";

  // Write crash history
  file << "[CrashHistory]\n";
  for (const auto& pair : crash_history_) {
    const auto& crash = pair.second;
    file << fmt::format("0x{:X}|{}|{}|{}|{}\n", crash.address,
                        static_cast<int>(crash.type), crash.frequency,
                        crash.timestamp, crash.details);
  }
  file << "\n";

  // Write workarounds
  file << "[Workarounds]\n";
  for (const auto& pair : workarounds_) {
    const auto& wa = pair.second;
    file << fmt::format("0x{:X}|{}|{}|{}|{}\n", wa.address,
                        static_cast<int>(wa.strategy), wa.times_applied,
                        wa.enabled ? 1 : 0, wa.reason);
  }
  file << "\n";

  // Write blacklisted addresses
  file << "[Blacklist]\n";
  for (uint64_t addr : blacklisted_addresses_) {
    file << fmt::format("0x{:X}\n", addr);
  }

  file.close();
  XELOGI("Learning database saved: {}", learning_db_path_);
}

void CrashRecoveryManager::LoadLearningDatabase() {
  if (learning_db_path_.empty()) {
    return;
  }

  std::ifstream file(learning_db_path_);
  if (!file.is_open()) {
    XELOGI("No existing learning database found (will create new)");
    return;
  }

  std::string line;
  std::string section;

  while (std::getline(file, line)) {
    // Skip comments and empty lines
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // Check for section headers
    if (line[0] == '[') {
      section = line.substr(1, line.find(']') - 1);
      continue;
    }

    // Parse based on section
    if (section == "CrashHistory") {
      // Parse crash info: address|type|frequency|timestamp|details
      std::istringstream iss(line);
      std::string addr_str, type_str, freq_str, ts_str;
      std::getline(iss, addr_str, '|');
      std::getline(iss, type_str, '|');
      std::getline(iss, freq_str, '|');
      std::getline(iss, ts_str, '|');

      CrashInfo crash;
      crash.address = std::stoull(addr_str, nullptr, 16);
      crash.type = static_cast<CrashType>(std::stoi(type_str));
      crash.frequency = std::stoul(freq_str);
      crash.timestamp = std::stoull(ts_str);
      std::getline(iss, crash.details);

      crash_history_[crash.address] = crash;
    } else if (section == "Workarounds") {
      // Parse workaround: address|strategy|times|enabled|reason
      std::istringstream iss(line);
      std::string addr_str, strat_str, times_str, enabled_str;
      std::getline(iss, addr_str, '|');
      std::getline(iss, strat_str, '|');
      std::getline(iss, times_str, '|');
      std::getline(iss, enabled_str, '|');

      Workaround wa;
      wa.address = std::stoull(addr_str, nullptr, 16);
      wa.strategy = static_cast<WorkaroundStrategy>(std::stoi(strat_str));
      wa.times_applied = std::stoul(times_str);
      wa.enabled = std::stoi(enabled_str) != 0;
      std::getline(iss, wa.reason);

      workarounds_[wa.address] = wa;
    } else if (section == "Blacklist") {
      uint64_t addr = std::stoull(line, nullptr, 16);
      blacklisted_addresses_.insert(addr);
    }
  }

  file.close();
  XELOGI("Learning database loaded from: {}", learning_db_path_);
}

uint64_t CrashRecoveryManager::GetCurrentTimestamp() const {
  return std::chrono::system_clock::now().time_since_epoch().count();
}

void CrashRecoveryManager::AnalyzeCrashPatterns() {
  if (!learning_enabled_) {
    return;
  }

  // Find patterns in crashes
  XELOGI("Analyzing crash patterns...");

  // Find addresses that crash very frequently
  for (const auto& pair : crash_history_) {
    if (pair.second.frequency >= 5 &&
        workarounds_.find(pair.first) == workarounds_.end()) {
      XELOGW("Frequent crash detected at 0x{:X} ({} times)", pair.first,
             pair.second.frequency);

      // Auto-apply aggressive workaround
      ApplyWorkaround(pair.first, WorkaroundStrategy::Skip,
                      "Auto-blacklisted due to high frequency");
      BlacklistAddress(pair.first, "Frequent crasher");
    }
  }
}

WorkaroundStrategy CrashRecoveryManager::DetermineWorkaround(
    const CrashInfo& crash) const {
  // Determine appropriate workaround based on crash type
  switch (crash.type) {
    case CrashType::MemoryAccess:
      return WorkaroundStrategy::ReturnZero;

    case CrashType::DivideByZero:
      return WorkaroundStrategy::ReturnZero;

    case CrashType::IllegalInstruction:
      return WorkaroundStrategy::Skip;

    case CrashType::StackOverflow:
      return WorkaroundStrategy::IgnoreError;

    case CrashType::GPUError:
      return WorkaroundStrategy::UseFallback;

    case CrashType::AudioError:
      return WorkaroundStrategy::IgnoreError;

    default:
      return WorkaroundStrategy::IgnoreError;
  }
}

}  // namespace crash_recovery
}  // namespace xe
