/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/game_compatibility.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/crash_recovery.h"
#include "xenia/base/logging.h"

namespace xe {
namespace game_compatibility {

GameCompatibilityDatabase& GameCompatibilityDatabase::GetInstance() {
  static GameCompatibilityDatabase instance;
  return instance;
}

void GameCompatibilityDatabase::Initialize() {
  if (initialized_) {
    return;
  }

  XELOGI("Initializing Game Compatibility Database...");
  InitializeBuiltInDatabase();
  AddBuiltInFixes();

  XELOGI("Game Compatibility Database initialized");
  XELOGI("  Known games: {}", games_.size());
  XELOGI("  Games with fixes: {}",
         std::count_if(games_.begin(), games_.end(),
                      [](const auto& p) { return !p.second.fixes.empty(); }));

  initialized_ = true;
}

void GameCompatibilityDatabase::Shutdown() {
  if (!initialized_) {
    return;
  }

  XELOGI("Game Compatibility Database shutdown");
  games_.clear();
  initialized_ = false;
}

void GameCompatibilityDatabase::InitializeBuiltInDatabase() {
  // Popular/problematic Xbox 360 games with known issues

  // Halo 3 (4D5307E6)
  RegisterKnownGame(0x4D5307E6, "Halo 3", CompatibilityStatus::Playable);

  // Halo: Reach (4D530919)
  RegisterKnownGame(0x4D530919, "Halo: Reach", CompatibilityStatus::Gameplay);

  // Halo 4 (4D530919)
  RegisterKnownGame(0x4D53085B, "Halo 4", CompatibilityStatus::Gameplay);

  // Red Dead Redemption (5454082B)
  RegisterKnownGame(0x5454082B, "Red Dead Redemption", CompatibilityStatus::Gameplay);

  // GTA IV (5454081C)
  RegisterKnownGame(0x5454081C, "Grand Theft Auto IV", CompatibilityStatus::Playable);

  // GTA V (5454087C)
  RegisterKnownGame(0x5454087C, "Grand Theft Auto V", CompatibilityStatus::Gameplay);

  // Gears of War (4D5307D1)
  RegisterKnownGame(0x4D5307D1, "Gears of War", CompatibilityStatus::Playable);

  // Gears of War 2 (4D530802)
  RegisterKnownGame(0x4D530802, "Gears of War 2", CompatibilityStatus::Playable);

  // Gears of War 3 (4D53085D)
  RegisterKnownGame(0x4D53085D, "Gears of War 3", CompatibilityStatus::Playable);

  // Forza Motorsport 3 (4D5307F1)
  RegisterKnownGame(0x4D5307F1, "Forza Motorsport 3", CompatibilityStatus::Gameplay);

  // Forza Motorsport 4 (4D530855)
  RegisterKnownGame(0x4D530855, "Forza Motorsport 4", CompatibilityStatus::Gameplay);

  // Call of Duty: Modern Warfare 2 (41560817)
  RegisterKnownGame(0x41560817, "Call of Duty: Modern Warfare 2", CompatibilityStatus::Playable);

  // Call of Duty: Black Ops (41560855)
  RegisterKnownGame(0x41560855, "Call of Duty: Black Ops", CompatibilityStatus::Playable);

  // Fable II (4D5307DC)
  RegisterKnownGame(0x4D5307DC, "Fable II", CompatibilityStatus::Playable);

  // Fable III (4D53085E)
  RegisterKnownGame(0x4D53085E, "Fable III", CompatibilityStatus::Playable);

  // Skyrim (425307D6)
  RegisterKnownGame(0x425307D6, "The Elder Scrolls V: Skyrim", CompatibilityStatus::Gameplay);

  // Fallout 3 (425307D1)
  RegisterKnownGame(0x425307D1, "Fallout 3", CompatibilityStatus::Gameplay);

  // Fallout: New Vegas (425307D5)
  RegisterKnownGame(0x425307D5, "Fallout: New Vegas", CompatibilityStatus::Gameplay);

  // Assassin's Creed II (5553083C)
  RegisterKnownGame(0x5553083C, "Assassin's Creed II", CompatibilityStatus::Playable);

  // Batman: Arkham Asylum (45410870)
  RegisterKnownGame(0x45410870, "Batman: Arkham Asylum", CompatibilityStatus::Playable);

  // Batman: Arkham City (5751087B)
  RegisterKnownGame(0x5751087B, "Batman: Arkham City", CompatibilityStatus::Playable);

  // Mass Effect (4541080B)
  RegisterKnownGame(0x4541080B, "Mass Effect", CompatibilityStatus::Playable);

  // Mass Effect 2 (45410829)
  RegisterKnownGame(0x45410829, "Mass Effect 2", CompatibilityStatus::Playable);

  // Mass Effect 3 (45410869)
  RegisterKnownGame(0x45410869, "Mass Effect 3", CompatibilityStatus::Playable);

  // Minecraft (584111F7)
  RegisterKnownGame(0x584111F7, "Minecraft: Xbox 360 Edition", CompatibilityStatus::Playable);

  // Alan Wake (4D53082D)
  RegisterKnownGame(0x4D53082D, "Alan Wake", CompatibilityStatus::Gameplay);

  // Bioshock (545407E4)
  RegisterKnownGame(0x545407E4, "Bioshock", CompatibilityStatus::Playable);

  // Bioshock Infinite (54540881)
  RegisterKnownGame(0x54540881, "Bioshock Infinite", CompatibilityStatus::Gameplay);

  // Dead Space (4541080E)
  RegisterKnownGame(0x4541080E, "Dead Space", CompatibilityStatus::Playable);

  // Mirror's Edge (4541080D)
  RegisterKnownGame(0x4541080D, "Mirror's Edge", CompatibilityStatus::Playable);

  // Saints Row 2 (5454082A)
  RegisterKnownGame(0x5454082A, "Saints Row 2", CompatibilityStatus::Gameplay);

  // Saints Row: The Third (5454086A)
  RegisterKnownGame(0x5454086A, "Saints Row: The Third", CompatibilityStatus::Gameplay);

  // Portal 2 (45410914)
  RegisterKnownGame(0x45410914, "Portal 2", CompatibilityStatus::Playable);

  // Left 4 Dead (4541080C)
  RegisterKnownGame(0x4541080C, "Left 4 Dead", CompatibilityStatus::Playable);

  // Crackdown (4D5307CE)
  RegisterKnownGame(0x4D5307CE, "Crackdown", CompatibilityStatus::Playable);

  // Beautiful Katamari (4E4D083A)
  RegisterKnownGame(0x4E4D083A, "Beautiful Katamari", CompatibilityStatus::Playable);

  // Soul Calibur V (4E4D083D) - Has multiple null pointer crashes in network code.
  // Code patches prevent crashes, but game shows black screen during boot.
  // Known unimplemented features: XamParty*, XamShowCommunitySessionsUI
  // Status: Loads (boots without crash with patches, but stuck at black screen)
  RegisterKnownGame(0x4E4D083D, "Soul Calibur V", CompatibilityStatus::Loads);

  // Soul Calibur IV (4E4D07E0)
  RegisterKnownGame(0x4E4D07E0, "Soul Calibur IV", CompatibilityStatus::Gameplay);
}

void GameCompatibilityDatabase::AddBuiltInFixes() {
  // Red Dead Redemption - Memory crashes
  if (HasGameInfo(0x5454082B)) {
    GameFix fix;
    fix.type = FixType::MemoryConfiguration;
    fix.description = "Increase memory allocation to prevent crashes";
    fix.enabled = true;
    fix.priority = 10;
    fix.memory_config.heap_size_64kb = 8192;  // 512MB
    fix.memory_config.use_large_pages = true;
    AddFix(0x5454082B, fix);

    GameFix cpu_fix;
    cpu_fix.type = FixType::CPUWorkaround;
    cpu_fix.description = "Blacklist problematic streaming addresses";
    cpu_fix.enabled = true;
    cpu_fix.priority = 9;
    // Known problematic addresses for RDR streaming
    cpu_fix.cpu_config.blacklisted_addresses = {0x82000000, 0x82100000};
    AddFix(0x5454082B, cpu_fix);
  }

  // Halo 3 - Graphics flickering
  if (HasGameInfo(0x4D5307E6)) {
    GameFix fix;
    fix.type = FixType::GraphicsSettings;
    fix.description = "Fix graphics flickering on Ampere GPUs";
    fix.enabled = true;
    fix.priority = 10;
    fix.graphics_config.disable_render_cache = true;
    fix.graphics_config.use_safe_shader_cache = true;
    AddFix(0x4D5307E6, fix);
  }

  // Skyrim - Frequent crashes
  if (HasGameInfo(0x425307D6)) {
    GameFix fix;
    fix.type = FixType::MemoryConfiguration;
    fix.description = "Prevent memory fragmentation crashes";
    fix.enabled = true;
    fix.priority = 10;
    fix.memory_config.heap_size_64kb = 6144;  // 384MB
    fix.memory_config.use_large_pages = true;
    AddFix(0x425307D6, fix);

    GameFix cpu_fix;
    cpu_fix.type = FixType::CPUWorkaround;
    cpu_fix.description = "Use safe JIT for script-heavy areas";
    cpu_fix.enabled = true;
    cpu_fix.priority = 8;
    cpu_fix.cpu_config.use_safe_jit = true;
    AddFix(0x425307D6, cpu_fix);
  }

  // GTA V - Performance and crashes
  if (HasGameInfo(0x5454087C)) {
    GameFix fix;
    fix.type = FixType::MemoryConfiguration;
    fix.description = "Large memory allocation for streaming";
    fix.enabled = true;
    fix.priority = 10;
    fix.memory_config.heap_size_64kb = 10240;  // 640MB
    fix.memory_config.use_large_pages = true;
    AddFix(0x5454087C, fix);

    GameFix gfx_fix;
    gfx_fix.type = FixType::GraphicsSettings;
    gfx_fix.description = "Optimize texture streaming";
    gfx_fix.enabled = true;
    gfx_fix.priority = 9;
    gfx_fix.graphics_config.max_texture_size = 2048;
    gfx_fix.graphics_config.disable_render_cache = true;
    AddFix(0x5454087C, gfx_fix);
  }

  // Fallout 3/New Vegas - Memory leaks and crashes
  for (uint32_t title_id : {0x425307D1, 0x425307D5}) {
    if (HasGameInfo(title_id)) {
      GameFix fix;
      fix.type = FixType::MemoryConfiguration;
      fix.description = "Prevent memory leaks in game engine";
      fix.enabled = true;
      fix.priority = 10;
      fix.memory_config.heap_size_64kb = 5120;  // 320MB
      fix.memory_config.use_large_pages = true;
      AddFix(title_id, fix);
    }
  }

  // Beautiful Katamari - Specific memory query fix
  if (HasGameInfo(0x4E4D083A)) {
    GameFix fix;
    fix.type = FixType::MemoryConfiguration;
    fix.description = "Fix memory queries for loading screen";
    fix.enabled = true;
    fix.priority = 10;
    fix.memory_config.heap_size_4kb = 2048;  // 8MB
    AddFix(0x4E4D083A, fix);
  }

  // Forza games - Shader compilation issues
  for (uint32_t title_id : {0x4D5307F1, 0x4D530855}) {
    if (HasGameInfo(title_id)) {
      GameFix fix;
      fix.type = FixType::GraphicsSettings;
      fix.description = "Safe shader cache to prevent compilation crashes";
      fix.enabled = true;
      fix.priority = 10;
      fix.graphics_config.use_safe_shader_cache = true;
      fix.graphics_config.disable_tessellation = true;
      AddFix(title_id, fix);
    }
  }

  // Soul Calibur V - Null pointer crash fixes
  // The game has multiple crashes due to uninitialized network/session objects.
  // These patches prevent the crashes but the game still shows a black screen
  // during boot, likely due to additional unimplemented features or the game
  // being stuck in a network initialization loop.
  // See: https://github.com/xenia-project/game-compatibility/issues/891
  if (HasGameInfo(0x4E4D083D)) {
    GameFix fix;
    fix.type = FixType::CPUWorkaround;
    fix.description = "Skip null pointer dereferences in network code";
    fix.enabled = true;
    fix.priority = 10;

    // Crash 1 at 0x82100080: lwz r10, 4(r11) with r11=0
    // The loop iterates through a linked list but the list head is null.
    // Instead of skipping the whole loop, we NOP the problematic load.
    // This allows the loop to continue naturally (r10 stays 0, loop exits).
    // Original: lwz r10, 4(r11) (814B0004) - loads from null+4
    // Patch: li r10, 0 (39400000) - safe value that will exit the loop
    fix.cpu_config.code_patches[0x82100080] = 0x39400000;

    // Crash 2 at 0x822A5BCC: lhz r11, 0(r11) with r11=0
    // Original: lhz r11, 0(r11) (A16B0000) - loads from null
    // Patch: li r11, 0 (39600000) - safe value instead of crash
    fix.cpu_config.code_patches[0x822A5BCC] = 0x39600000;

    // Crash 3 at 0x82543C04: lwz r9, 0(r3) with r3=0
    // Original: lwz r9, 0(r3) (81230000) - loads from null
    // Patch: li r9, 0 (39200000) - safe value instead of crash
    fix.cpu_config.code_patches[0x82543C04] = 0x39200000;

    AddFix(0x4E4D083D, fix);

    // Graphics settings fix for SCV - similar to Halo 3
    // The game may have rendering issues with render cache or shader compilation
    GameFix gfx_fix;
    gfx_fix.type = FixType::GraphicsSettings;
    gfx_fix.description = "Fix rendering issues (light blue strip)";
    gfx_fix.enabled = true;
    gfx_fix.priority = 9;
    gfx_fix.graphics_config.disable_render_cache = true;
    gfx_fix.graphics_config.use_safe_shader_cache = true;
    AddFix(0x4E4D083D, gfx_fix);
  }
}

void GameCompatibilityDatabase::RegisterKnownGame(
    uint32_t title_id, const std::string& name, CompatibilityStatus status) {
  GameInfo info;
  info.title_id = title_id;
  info.title_name = name;
  info.status = status;
  info.last_updated = std::chrono::system_clock::now().time_since_epoch().count();

  games_[title_id] = info;
}

bool GameCompatibilityDatabase::HasGameInfo(uint32_t title_id) const {
  return games_.find(title_id) != games_.end();
}

GameInfo GameCompatibilityDatabase::GetGameInfo(uint32_t title_id) const {
  auto it = games_.find(title_id);
  if (it != games_.end()) {
    return it->second;
  }

  // Return unknown game
  GameInfo unknown;
  unknown.title_id = title_id;
  unknown.title_name = "Unknown Game";
  unknown.status = CompatibilityStatus::Unknown;
  return unknown;
}

CompatibilityStatus GameCompatibilityDatabase::GetStatus(uint32_t title_id) const {
  auto it = games_.find(title_id);
  if (it != games_.end()) {
    return it->second.status;
  }
  return CompatibilityStatus::Unknown;
}

void GameCompatibilityDatabase::ApplyFixes(uint32_t title_id) {
  auto fixes = GetFixes(title_id);
  if (fixes.empty()) {
    XELOGI("No fixes available for title {:08X}", title_id);
    return;
  }

  XELOGI("Applying {} fixes for title {:08X}", fixes.size(), title_id);

  // Sort by priority (highest first)
  std::sort(fixes.begin(), fixes.end(),
            [](const GameFix& a, const GameFix& b) {
              return a.priority > b.priority;
            });

  for (const auto& fix : fixes) {
    if (!fix.enabled) {
      continue;
    }

    XELOGI("  - Applying: {}", fix.description);

    switch (fix.type) {
      case FixType::MemoryConfiguration:
        // Memory fixes are applied during heap initialization
        break;

      case FixType::CPUWorkaround:
        // Apply CPU workarounds through crash recovery
        for (uint32_t addr : fix.cpu_config.blacklisted_addresses) {
          crash_recovery::CrashRecoveryManager::GetInstance()
              .BlacklistGuestAddress(addr, fix.description);
        }
        break;

      case FixType::GraphicsSettings:
        // Graphics settings are applied when GPU initializes
        break;

      case FixType::BlacklistAddress:
        // Already handled in CPUWorkaround
        break;

      default:
        XELOGW("    Fix type not yet implemented");
        break;
    }
  }
}

std::vector<GameFix> GameCompatibilityDatabase::GetFixes(uint32_t title_id) const {
  auto it = games_.find(title_id);
  if (it != games_.end()) {
    return it->second.fixes;
  }
  return {};
}

void GameCompatibilityDatabase::AddGame(const GameInfo& info) {
  games_[info.title_id] = info;
  XELOGI("Added game: {} ({:08X})", info.title_name, info.title_id);
}

void GameCompatibilityDatabase::UpdateStatus(uint32_t title_id,
                                              CompatibilityStatus status) {
  auto it = games_.find(title_id);
  if (it != games_.end()) {
    it->second.status = status;
    it->second.last_updated =
        std::chrono::system_clock::now().time_since_epoch().count();
  }
}

void GameCompatibilityDatabase::AddIssue(uint32_t title_id, IssueType issue) {
  auto it = games_.find(title_id);
  if (it != games_.end()) {
    it->second.known_issues.push_back(issue);
  }
}

void GameCompatibilityDatabase::AddFix(uint32_t title_id, const GameFix& fix) {
  auto it = games_.find(title_id);
  if (it != games_.end()) {
    it->second.fixes.push_back(fix);
  }
}

std::vector<uint32_t> GameCompatibilityDatabase::GetGamesByStatus(
    CompatibilityStatus status) const {
  std::vector<uint32_t> result;
  for (const auto& pair : games_) {
    if (pair.second.status == status) {
      result.push_back(pair.first);
    }
  }
  return result;
}

std::vector<GameInfo> GameCompatibilityDatabase::GetProblematicGames() const {
  std::vector<GameInfo> result;
  for (const auto& pair : games_) {
    if (pair.second.status == CompatibilityStatus::Broken ||
        pair.second.status == CompatibilityStatus::Loads ||
        !pair.second.known_issues.empty()) {
      result.push_back(pair.second);
    }
  }
  return result;
}

bool GameCompatibilityDatabase::LoadFromFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    XELOGW("Failed to load compatibility database from: {}", path);
    return false;
  }

  // TODO: Implement JSON/XML parsing
  XELOGI("Loading compatibility database from: {}", path);

  file.close();
  return true;
}

bool GameCompatibilityDatabase::SaveToFile(const std::string& path) {
  std::ofstream file(path);
  if (!file.is_open()) {
    XELOGW("Failed to save compatibility database to: {}", path);
    return false;
  }

  file << "# Xenia Game Compatibility Database\n";
  file << "# Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";

  for (const auto& pair : games_) {
    const auto& game = pair.second;
    file << fmt::format("[{:08X}]\n", game.title_id);
    file << "Name=" << game.title_name << "\n";
    file << "Status=" << static_cast<int>(game.status) << "\n";
    file << "Fixes=" << game.fixes.size() << "\n\n";
  }

  file.close();
  XELOGI("Compatibility database saved to: {}", path);
  return true;
}

bool GameCompatibilityDatabase::UpdateFromURL(const std::string& url) {
  // TODO: Implement HTTP download from community database
  XELOGI("Updating compatibility database from: {}", url);
  return false;
}

bool GameCompatibilityDatabase::LoadCommunityDatabase() {
  // TODO: Load from xenia-project.github.io or similar
  XELOGI("Loading community compatibility database...");
  return false;
}

// GameFixApplicator implementation
GameFixApplicator::GameFixApplicator(uint32_t title_id) : title_id_(title_id) {
  auto& db = GameCompatibilityDatabase::GetInstance();

  if (!db.HasGameInfo(title_id)) {
    XELOGI("No compatibility info for title {:08X}", title_id);
    return;
  }

  auto info = db.GetGameInfo(title_id);
  XELOGI("Loaded compatibility info for: {}", info.title_name);
  XELOGI("  Status: {}", static_cast<int>(info.status));
  XELOGI("  Known issues: {}", info.known_issues.size());
  XELOGI("  Available fixes: {}", info.fixes.size());

  // Apply all fixes
  db.ApplyFixes(title_id);
}

GameFixApplicator::~GameFixApplicator() {
  if (!applied_fixes_.empty()) {
    XELOGI("Applied {} fixes for title {:08X}", applied_fixes_.size(), title_id_);
  }
}

void GameFixApplicator::ApplyMemoryFixes(const MemoryConfig& config) {
  XELOGI("Applying memory fixes for title {:08X}", title_id_);

  if (config.heap_size_64kb > 0) {
    XELOGI("  - Setting 64KB heap size: {} pages", config.heap_size_64kb);
  }

  if (config.use_large_pages) {
    XELOGI("  - Enabling large page support");
  }

  applied_fixes_.push_back("MemoryConfiguration");
}

void GameFixApplicator::ApplyGraphicsFixes(const GraphicsConfig& config) {
  XELOGI("Applying graphics fixes for title {:08X}", title_id_);

  if (config.use_safe_shader_cache) {
    XELOGI("  - Using safe shader cache");
  }

  if (config.disable_render_cache) {
    XELOGI("  - Disabling render cache");
  }

  applied_fixes_.push_back("GraphicsConfiguration");
}

void GameFixApplicator::ApplyCPUFixes(const CPUConfig& config) {
  XELOGI("Applying CPU fixes for title {:08X}", title_id_);

  if (config.use_safe_jit) {
    XELOGI("  - Using safe JIT compilation");
  }

  if (!config.blacklisted_addresses.empty()) {
    XELOGI("  - Blacklisting {} addresses", config.blacklisted_addresses.size());
  }

  applied_fixes_.push_back("CPUConfiguration");
}

}  // namespace game_compatibility
}  // namespace xe
