/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2024 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_APP_UPDATE_CHECKER_H_
#define XENIA_APP_UPDATE_CHECKER_H_

#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace xe {
namespace app {

// Represents information about an available update
struct UpdateInfo {
  std::string version;
  std::string download_url;
  std::string release_notes;
  std::string published_at;
  bool is_newer = false;
};

// Callback type for update check completion
using UpdateCheckCallback = std::function<void(bool success, const UpdateInfo& info)>;

// Checks for updates from GitHub releases
class UpdateChecker {
 public:
  UpdateChecker();
  ~UpdateChecker();

  // Check for updates asynchronously
  // The callback will be invoked on a background thread
  void CheckForUpdatesAsync(UpdateCheckCallback callback);

  // Check for updates synchronously (blocking)
  bool CheckForUpdates(UpdateInfo& info);

  // Get the URL to open for downloading the latest release
  static std::string GetReleasesUrl();

  // Get the current build version string
  static std::string GetCurrentVersion();

 private:
  // Parse the GitHub API response JSON
  bool ParseReleaseInfo(const std::string& json_response, UpdateInfo& info);

  // Compare version strings (returns true if remote is newer)
  bool IsNewerVersion(const std::string& current, const std::string& remote);

  // Perform HTTP GET request
  bool HttpGet(const std::string& url, std::string& response);

  std::unique_ptr<std::thread> check_thread_;
};

}  // namespace app
}  // namespace xe

#endif  // XENIA_APP_UPDATE_CHECKER_H_
