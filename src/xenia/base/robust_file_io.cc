/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/robust_file_io.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <thread>

#include "third_party/fmt/include/fmt/format.h"
#include "xenia/base/filesystem.h"
#include "xenia/base/logging.h"
#include "xenia/base/string.h"

#if XE_PLATFORM_WIN32
#include <Windows.h>
#endif

namespace xe {
namespace robust_io {

namespace {

// CRC32 lookup table
uint32_t crc32_table[256];
bool crc32_table_initialized = false;

void InitializeCRC32Table() {
  if (crc32_table_initialized) return;

  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;
    for (int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }
    crc32_table[i] = crc;
  }
  crc32_table_initialized = true;
}

uint32_t CalculateCRC32(const uint8_t* data, size_t length) {
  InitializeCRC32Table();

  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++) {
    crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
  }
  return ~crc;
}

uint64_t GetCurrentTimeMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  // namespace

// RobustFileReader implementation
RobustFileReader::RobustFileReader(const RobustIOConfig& config)
    : config_(config),
      total_retries_(0),
      interference_count_(0),
      recovered_errors_(0) {}

RobustFileReader::~RobustFileReader() {
  if (total_retries_ > 0 || interference_count_ > 0) {
    XELOGI("RobustFileReader statistics:");
    XELOGI("  Total retries: {}", total_retries_);
    XELOGI("  Interference detections: {}", interference_count_);
    XELOGI("  Recovered errors: {}", recovered_errors_);
  }
}

IOResult RobustFileReader::ReadFile(const std::filesystem::path& path,
                                    std::vector<uint8_t>& data) {
  XELOGI("Reading file: {}", xe::path_to_utf8(path));

  // First verify the file is accessible
  auto verify_result = VerifyFileAccess(path);
  if (!verify_result.IsSuccess()) {
    return verify_result;
  }

  // Try to read with retry logic
  return ReadWithRetry(path, data);
}

IOResult RobustFileReader::ReadFileChunked(
    const std::filesystem::path& path, std::vector<uint8_t>& data,
    std::function<void(size_t bytes_read, size_t total_size)> progress_cb) {

  XELOGI("Reading file in chunks: {}", xe::path_to_utf8(path));

  auto start_time = GetCurrentTimeMs();

  try {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      return {IOErrorType::FileNotFound,
              fmt::format("Could not open file: {}", xe::path_to_utf8(path)), 0,
              0, false};
    }

    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    data.resize(file_size);
    size_t bytes_read = 0;
    size_t chunk_size = config_.read_chunk_size;

    while (bytes_read < file_size) {
      size_t to_read = std::min(chunk_size, file_size - bytes_read);

      auto chunk_start = GetCurrentTimeMs();
      file.read(reinterpret_cast<char*>(data.data() + bytes_read), to_read);
      auto chunk_duration = GetCurrentTimeMs() - chunk_start;

      if (!file.good() && !file.eof()) {
        XELOGE("Read error at offset {} / {}", bytes_read, file_size);

        // Detect interference
        if (DetectInterference(chunk_duration, to_read)) {
          interference_count_++;
          XELOGW("Interference detected during read!");
        }

        return {IOErrorType::ReadError,
                fmt::format("Read failed at offset {}", bytes_read), bytes_read,
                0, false};
      }

      bytes_read += file.gcount();

      // Report progress
      if (progress_cb) {
        progress_cb(bytes_read, file_size);
      }

      // Check for interference
      if (config_.detect_interference &&
          DetectInterference(chunk_duration, to_read)) {
        interference_count_++;
        XELOGW("Interference detected! Read took {}ms for {} bytes",
               chunk_duration, to_read);
        InterferenceDetector::GetInstance().RecordIOTiming(chunk_duration,
                                                           to_read);
      }
    }

    auto total_duration = GetCurrentTimeMs() - start_time;
    XELOGI("Read {} bytes in {}ms ({:.2f} MB/s)", file_size, total_duration,
           (file_size / 1024.0 / 1024.0) / (total_duration / 1000.0));

    return {IOErrorType::Success, "File read successfully", bytes_read, 0,
            false};

  } catch (const std::exception& e) {
    return {IOErrorType::Unknown, fmt::format("Exception: {}", e.what()), 0, 0,
            false};
  }
}

IOResult RobustFileReader::ReadFileVerified(const std::filesystem::path& path,
                                            std::vector<uint8_t>& data,
                                            uint32_t expected_crc) {
  auto result = ReadFile(path, data);
  if (!result.IsSuccess()) {
    return result;
  }

  // Verify CRC
  uint32_t actual_crc = CalculateCRC32(data);
  if (actual_crc != expected_crc) {
    XELOGE("CRC mismatch! Expected {:08X}, got {:08X}", expected_crc,
           actual_crc);
    return {IOErrorType::ChecksumMismatch,
            fmt::format("CRC mismatch: expected {:08X}, got {:08X}",
                        expected_crc, actual_crc),
            data.size(), 0, false};
  }

  XELOGI("CRC verification passed: {:08X}", actual_crc);
  return result;
}

IOResult RobustFileReader::VerifyFileAccess(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    return {IOErrorType::FileNotFound,
            fmt::format("File not found: {}", xe::path_to_utf8(path)), 0, 0,
            false};
  }

#if XE_PLATFORM_WIN32
  // Check if file is locked or in use
  HANDLE handle = CreateFileW(
      path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    if (error == ERROR_SHARING_VIOLATION) {
      return {IOErrorType::AccessDenied,
              "File is locked by another process", 0, 0, false};
    } else if (error == ERROR_NOT_READY || error == ERROR_DEVICE_NOT_AVAILABLE) {
      return {IOErrorType::DeviceNotReady, "Device not ready", 0, 0, false};
    }
    return {IOErrorType::AccessDenied, "Cannot access file", 0, 0, false};
  }

  CloseHandle(handle);
#endif

  return {IOErrorType::Success, "File is accessible", 0, 0, false};
}

IOResult RobustFileReader::ReadWithRetry(const std::filesystem::path& path,
                                         std::vector<uint8_t>& data) {
  IOResult last_result;

  for (int retry = 0; retry <= config_.max_retries; retry++) {
    if (retry > 0) {
      XELOGW("Retry attempt {} of {}", retry, config_.max_retries);
      total_retries_++;
      WaitBeforeRetry(retry);
    }

    try {
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      if (!file.is_open()) {
        last_result = {IOErrorType::FileNotFound, "Could not open file", 0,
                       retry, false};
        continue;
      }

      size_t file_size = file.tellg();
      file.seekg(0, std::ios::beg);

      data.resize(file_size);

      auto start_time = GetCurrentTimeMs();
      file.read(reinterpret_cast<char*>(data.data()), file_size);
      auto duration = GetCurrentTimeMs() - start_time;

      if (!file.good() && !file.eof()) {
        XELOGE("Read error occurred");
        last_result = {IOErrorType::ReadError, "Read operation failed", 0,
                       retry, false};

        // Detect interference
        if (DetectInterference(duration, file_size)) {
          interference_count_++;
          last_result.error = IOErrorType::InterferenceDetected;
        }
        continue;
      }

      size_t bytes_read = file.gcount();
      if (bytes_read != file_size) {
        XELOGW("Partial read: {} of {} bytes", bytes_read, file_size);
        last_result = {IOErrorType::PartialRead,
                       fmt::format("Read {} of {} bytes", bytes_read, file_size),
                       bytes_read, retry, false};
        continue;
      }

      // Success!
      XELOGI("Successfully read {} bytes", bytes_read);
      if (retry > 0) {
        recovered_errors_++;
        XELOGI("Recovered after {} retries", retry);
      }

      return {IOErrorType::Success, "File read successfully", bytes_read, retry,
              retry > 0};

    } catch (const std::exception& e) {
      XELOGE("Exception during read: {}", e.what());
      last_result = {IOErrorType::Unknown, e.what(), 0, retry, false};
    }
  }

  // All retries exhausted
  XELOGE("Failed to read file after {} retries", config_.max_retries);
  return last_result;
}

IOResult RobustFileReader::VerifyData(const std::vector<uint8_t>& data,
                                      size_t expected_size) {
  if (data.size() != expected_size) {
    return {IOErrorType::CorruptedData,
            fmt::format("Size mismatch: expected {}, got {}", expected_size,
                        data.size()),
            data.size(), 0, false};
  }

  return {IOErrorType::Success, "Data verification passed", data.size(), 0,
          false};
}

uint32_t RobustFileReader::CalculateCRC32(const std::vector<uint8_t>& data) {
  return ::xe::robust_io::CalculateCRC32(data.data(), data.size());
}

bool RobustFileReader::DetectInterference(uint64_t read_time_ms,
                                          size_t bytes_read) {
  if (!config_.detect_interference) {
    return false;
  }

  // Calculate expected time (assuming reasonable HDD speed of 100 MB/s)
  double expected_time_ms = (bytes_read / (100.0 * 1024.0 * 1024.0)) * 1000.0;

  // If actual time is much longer than expected, interference is likely
  if (read_time_ms > expected_time_ms * 5 &&
      read_time_ms > config_.interference_threshold_ms) {
    XELOGW("Interference detected: {}ms for {} bytes (expected ~{}ms)",
           read_time_ms, bytes_read, static_cast<int>(expected_time_ms));
    return true;
  }

  return false;
}

void RobustFileReader::WaitBeforeRetry(int retry_count) {
  int delay_ms = config_.retry_delay_ms;

  if (config_.exponential_backoff) {
    delay_ms = config_.retry_delay_ms * (1 << (retry_count - 1));
    delay_ms = std::min(delay_ms, 5000);  // Max 5 seconds
  }

  XELOGI("Waiting {}ms before retry...", delay_ms);
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

// InterferenceDetector implementation
InterferenceDetector& InterferenceDetector::GetInstance() {
  static InterferenceDetector instance;
  return instance;
}

InterferenceDetector::InterferenceDetector()
    : avg_io_time_ms_(0),
      interference_count_(0),
      current_level_(InterferenceLevel::None) {}

InterferenceDetector::~InterferenceDetector() = default;

InterferenceDetector::InterferenceLevel
InterferenceDetector::DetectCurrentLevel() {
  if (recent_samples_.empty()) {
    return InterferenceLevel::None;
  }

  // Calculate average I/O time from recent samples
  uint64_t total_time = 0;
  for (const auto& sample : recent_samples_) {
    total_time += sample.duration_ms;
  }
  avg_io_time_ms_ = total_time / recent_samples_.size();

  // Determine interference level
  if (avg_io_time_ms_ < 100) {
    current_level_ = InterferenceLevel::None;
  } else if (avg_io_time_ms_ < 300) {
    current_level_ = InterferenceLevel::Low;
  } else if (avg_io_time_ms_ < 1000) {
    current_level_ = InterferenceLevel::Medium;
  } else if (avg_io_time_ms_ < 3000) {
    current_level_ = InterferenceLevel::High;
  } else {
    current_level_ = InterferenceLevel::Critical;
  }

  return current_level_;
}

void InterferenceDetector::RecordIOTiming(uint64_t duration_ms, size_t bytes) {
  IOSample sample;
  sample.timestamp = GetCurrentTimeMs();
  sample.duration_ms = duration_ms;
  sample.bytes = bytes;

  recent_samples_.push_back(sample);

  // Keep only last 20 samples
  if (recent_samples_.size() > 20) {
    recent_samples_.erase(recent_samples_.begin());
  }

  // Update interference detection
  DetectCurrentLevel();
}

bool InterferenceDetector::IsInterferenceActive() const {
  return current_level_ >= InterferenceLevel::Medium;
}

std::string InterferenceDetector::GetMitigationAdvice() const {
  switch (current_level_) {
    case InterferenceLevel::None:
      return "No interference detected";
    case InterferenceLevel::Low:
      return "Minor interference - performance may be slightly affected";
    case InterferenceLevel::Medium:
      return "Moderate interference - try disabling Bluetooth/WiFi or moving "
             "phone away";
    case InterferenceLevel::High:
      return "High interference - move phone away from PC, disable wireless "
             "devices";
    case InterferenceLevel::Critical:
      return "Critical interference - check USB connections, wireless devices, "
             "and phone proximity";
  }
  return "Unknown";
}

// Helper functions
namespace helpers {

IOResult LoadGameFile(const std::filesystem::path& path,
                      std::vector<uint8_t>& data) {
  RobustIOConfig config;
  config.max_retries = 5;
  config.detect_interference = true;
  config.verify_file_size = true;

  RobustFileReader reader(config);
  return reader.ReadFileChunked(path, data);
}

IOResult LoadGameFileWithProgress(
    const std::filesystem::path& path, std::vector<uint8_t>& data,
    std::function<void(int progress_percent)> progress_cb) {

  RobustIOConfig config;
  config.max_retries = 5;
  config.detect_interference = true;

  RobustFileReader reader(config);

  return reader.ReadFileChunked(
      path, data, [progress_cb](size_t bytes_read, size_t total_size) {
        if (progress_cb && total_size > 0) {
          int percent = static_cast<int>((bytes_read * 100) / total_size);
          progress_cb(percent);
        }
      });
}

bool IsFileCorrupted(const std::filesystem::path& path) {
  // Try to read the file
  RobustFileReader reader;
  std::vector<uint8_t> data;
  auto result = reader.ReadFile(path, data);

  return !result.IsSuccess();
}

IOResult RepairFile(const std::filesystem::path& path) {
  // For now, just attempt to read with maximum retry attempts
  RobustIOConfig config;
  config.max_retries = 10;
  config.retry_delay_ms = 200;
  config.exponential_backoff = true;

  RobustFileReader reader(config);
  std::vector<uint8_t> data;
  return reader.ReadFile(path, data);
}

}  // namespace helpers

}  // namespace robust_io
}  // namespace xe
