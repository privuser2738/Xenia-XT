/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2024 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/app/update_checker.h"

#include <regex>

#include "build/version.h"
#include "xenia/base/logging.h"

#if XE_PLATFORM_WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

namespace xe {
namespace app {

namespace {
const char* kGitHubApiUrl = "api.github.com";
const char* kReleasesPath = "/repos/xenia-project/xenia/releases/latest";
const char* kReleasesPageUrl = "https://github.com/xenia-project/xenia/releases";
}  // namespace

UpdateChecker::UpdateChecker() = default;

UpdateChecker::~UpdateChecker() {
  if (check_thread_ && check_thread_->joinable()) {
    check_thread_->join();
  }
}

void UpdateChecker::CheckForUpdatesAsync(UpdateCheckCallback callback) {
  // Clean up previous thread if any
  if (check_thread_ && check_thread_->joinable()) {
    check_thread_->join();
  }

  check_thread_ = std::make_unique<std::thread>([this, callback]() {
    UpdateInfo info;
    bool success = CheckForUpdates(info);
    if (callback) {
      callback(success, info);
    }
  });
}

bool UpdateChecker::CheckForUpdates(UpdateInfo& info) {
  std::string response;
  std::string url = std::string("https://") + kGitHubApiUrl + kReleasesPath;

  if (!HttpGet(url, response)) {
    XELOGE("UpdateChecker: Failed to fetch release information");
    return false;
  }

  if (!ParseReleaseInfo(response, info)) {
    XELOGE("UpdateChecker: Failed to parse release information");
    return false;
  }

  info.is_newer = IsNewerVersion(GetCurrentVersion(), info.version);
  XELOGI("UpdateChecker: Current version: {}, Latest version: {}, Update available: {}",
         GetCurrentVersion(), info.version, info.is_newer);

  return true;
}

std::string UpdateChecker::GetReleasesUrl() {
  return kReleasesPageUrl;
}

std::string UpdateChecker::GetCurrentVersion() {
  // Use the build commit hash as the version identifier
  return XE_BUILD_COMMIT;
}

bool UpdateChecker::ParseReleaseInfo(const std::string& json_response, UpdateInfo& info) {
  // Simple JSON parsing without external dependencies
  // Look for "tag_name", "html_url", "body", "published_at"

  auto extract_field = [&json_response](const std::string& field) -> std::string {
    std::string pattern = "\"" + field + "\"\\s*:\\s*\"([^\"]+)\"";
    std::regex re(pattern);
    std::smatch match;
    if (std::regex_search(json_response, match, re) && match.size() > 1) {
      return match[1].str();
    }
    return "";
  };

  info.version = extract_field("tag_name");
  info.download_url = extract_field("html_url");
  info.published_at = extract_field("published_at");

  // Body field might contain escaped characters, handle separately
  std::regex body_re("\"body\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\"");
  std::smatch body_match;
  if (std::regex_search(json_response, body_match, body_re) && body_match.size() > 1) {
    info.release_notes = body_match[1].str();
    // Unescape common escape sequences
    std::regex newline_re("\\\\n");
    info.release_notes = std::regex_replace(info.release_notes, newline_re, "\n");
  }

  return !info.version.empty();
}

bool UpdateChecker::IsNewerVersion(const std::string& current, const std::string& remote) {
  // For commit-based versioning, we can't directly compare
  // If they're different and we successfully fetched, assume newer
  // A more sophisticated approach would compare commit dates or use semver
  if (current.empty() || remote.empty()) {
    return false;
  }

  // Simple check: if versions are different, assume remote is newer
  // This is a simplification - in production, you'd want to compare commit dates
  return current != remote;
}

bool UpdateChecker::HttpGet(const std::string& url, std::string& response) {
#if XE_PLATFORM_WIN32
  HINTERNET hSession = WinHttpOpen(L"Xenia-XT Update Checker/1.0",
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME,
                                    WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) {
    return false;
  }

  // Parse URL
  URL_COMPONENTS urlComponents = {};
  urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
  wchar_t hostName[256] = {};
  wchar_t urlPath[1024] = {};
  urlComponents.lpszHostName = hostName;
  urlComponents.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
  urlComponents.lpszUrlPath = urlPath;
  urlComponents.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

  std::wstring wurl(url.begin(), url.end());
  if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComponents)) {
    WinHttpCloseHandle(hSession);
    return false;
  }

  HINTERNET hConnect = WinHttpConnect(hSession, hostName,
                                       urlComponents.nPort, 0);
  if (!hConnect) {
    WinHttpCloseHandle(hSession);
    return false;
  }

  DWORD flags = (urlComponents.nScheme == INTERNET_SCHEME_HTTPS)
                ? WINHTTP_FLAG_SECURE : 0;
  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath,
                                           NULL, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           flags);
  if (!hRequest) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return false;
  }

  // Set User-Agent header (GitHub API requires it)
  WinHttpAddRequestHeaders(hRequest,
                           L"User-Agent: Xenia-Emulator\r\n",
                           -1L, WINHTTP_ADDREQ_FLAG_ADD);

  if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                          WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return false;
  }

  if (!WinHttpReceiveResponse(hRequest, NULL)) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return false;
  }

  // Read response
  DWORD bytesAvailable = 0;
  response.clear();

  do {
    bytesAvailable = 0;
    if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
      break;
    }

    if (bytesAvailable > 0) {
      std::vector<char> buffer(bytesAvailable + 1);
      DWORD bytesRead = 0;
      if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
        response.append(buffer.data(), bytesRead);
      }
    }
  } while (bytesAvailable > 0);

  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hSession);

  return !response.empty();
#else
  // Non-Windows platforms: return false for now
  // Could implement using libcurl or system curl command
  XELOGW("UpdateChecker: HTTP requests not implemented for this platform");
  return false;
#endif
}

}  // namespace app
}  // namespace xe
