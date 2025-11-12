/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_BASE_GAME_COMPATIBILITY_H_
#define XENIA_BASE_GAME_COMPATIBILITY_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <set>

namespace xe {
namespace game_compatibility {

// Game status levels
enum class CompatibilityStatus {
  Unknown,          // Not tested
  Broken,           // Doesn't boot or unplayable
  Loads,            // Loads but crashes frequently
  Gameplay,         // Playable with issues
  Playable,         // Playable with minor issues
  Perfect,          // Works perfectly
};

// Known issue types
enum class IssueType {
  GraphicsCorruption,
  AudioGlitches,
  MemoryLeak,
  FrequentCrashes,
  SlowPerformance,
  InputIssues,
  SaveGameIssues,
  NetworkingBroken,
  CutsceneIssues,
  PhysicsGlitches,
  TextureIssues,
  ShaderIssues,
};

// Fix/workaround types
enum class FixType {
  MemoryConfiguration,   // Adjust memory allocation
  GraphicsSettings,      // GPU-specific settings
  CPUWorkaround,         // CPU instruction workarounds
  TimingAdjustment,      // Timing/sync adjustments
  BlacklistAddress,      // Blacklist problematic code
  PatchCode,             // Patch game code
  SkipFunction,          // Skip problematic functions
  ForceSettings,         // Force specific settings
};

// Memory configuration for specific games
struct MemoryConfig {
  uint32_t heap_size_4kb = 0;      // Size in 4KB pages
  uint32_t heap_size_64kb = 0;     // Size in 64KB pages
  uint32_t heap_size_16mb = 0;     // Size in 16MB pages
  bool use_large_pages = false;
  bool disable_write_combine = false;
  std::vector<std::pair<uint32_t, uint32_t>> reserved_regions;  // start, size
};

// Graphics configuration
struct GraphicsConfig {
  bool disable_vsync = false;
  bool force_msaa = false;
  int msaa_samples = 4;
  bool disable_tessellation = false;
  bool use_safe_shader_cache = false;
  int max_texture_size = 4096;
  bool disable_render_cache = false;
};

// CPU configuration
struct CPUConfig {
  bool use_safe_jit = false;
  bool disable_fast_math = false;
  std::set<uint32_t> blacklisted_addresses;
  std::map<uint32_t, uint32_t> code_patches;  // address -> replacement
  std::set<std::string> disabled_functions;
};

// A specific fix/workaround
struct GameFix {
  FixType type;
  std::string description;
  bool enabled;
  int priority;  // Higher priority applied first

  // Type-specific data
  MemoryConfig memory_config;
  GraphicsConfig graphics_config;
  CPUConfig cpu_config;
};

// Information about a specific game
struct GameInfo {
  uint32_t title_id;
  std::string title_name;
  std::string region;
  CompatibilityStatus status;

  std::vector<IssueType> known_issues;
  std::vector<GameFix> fixes;

  std::string notes;
  std::string tested_version;
  uint64_t last_updated;
};

// Database of game compatibility information
class GameCompatibilityDatabase {
 public:
  static GameCompatibilityDatabase& GetInstance();

  // Initialize with built-in database
  void Initialize();
  void Shutdown();

  // Load/save external database
  bool LoadFromFile(const std::string& path);
  bool SaveToFile(const std::string& path);

  // Load from community sources
  bool UpdateFromURL(const std::string& url);
  bool LoadCommunityDatabase();

  // Query game information
  bool HasGameInfo(uint32_t title_id) const;
  GameInfo GetGameInfo(uint32_t title_id) const;
  CompatibilityStatus GetStatus(uint32_t title_id) const;

  // Apply fixes for a game
  void ApplyFixes(uint32_t title_id);
  std::vector<GameFix> GetFixes(uint32_t title_id) const;

  // Add/update game information
  void AddGame(const GameInfo& info);
  void UpdateStatus(uint32_t title_id, CompatibilityStatus status);
  void AddIssue(uint32_t title_id, IssueType issue);
  void AddFix(uint32_t title_id, const GameFix& fix);

  // Statistics
  size_t GetGameCount() const { return games_.size(); }
  std::vector<uint32_t> GetGamesByStatus(CompatibilityStatus status) const;

  // Get all games with issues
  std::vector<GameInfo> GetProblematicGames() const;

 private:
  GameCompatibilityDatabase() = default;
  ~GameCompatibilityDatabase() = default;

  // No copy/move
  GameCompatibilityDatabase(const GameCompatibilityDatabase&) = delete;
  GameCompatibilityDatabase& operator=(const GameCompatibilityDatabase&) = delete;

  void InitializeBuiltInDatabase();
  void RegisterKnownGame(uint32_t title_id, const std::string& name,
                         CompatibilityStatus status);
  void AddBuiltInFixes();

  std::map<uint32_t, GameInfo> games_;
  bool initialized_ = false;
};

// Helper to automatically apply fixes when a game is loaded
class GameFixApplicator {
 public:
  explicit GameFixApplicator(uint32_t title_id);
  ~GameFixApplicator();

  void ApplyMemoryFixes(const MemoryConfig& config);
  void ApplyGraphicsFixes(const GraphicsConfig& config);
  void ApplyCPUFixes(const CPUConfig& config);

 private:
  uint32_t title_id_;
  std::vector<std::string> applied_fixes_;
};

}  // namespace game_compatibility
}  // namespace xe

#endif  // XENIA_BASE_GAME_COMPATIBILITY_H_
