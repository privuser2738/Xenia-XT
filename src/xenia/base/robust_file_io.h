/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_BASE_ROBUST_FILE_IO_H_
#define XENIA_BASE_ROBUST_FILE_IO_H_

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace xe {
namespace robust_io {

// Error types that can occur during file operations
enum class IOErrorType {
  Success,
  FileNotFound,
  AccessDenied,
  ReadError,
  WriteError,
  CorruptedData,
  DeviceNotReady,
  DeviceRemoved,
  Timeout,
  InterferenceDetected,
  ChecksumMismatch,
  PartialRead,
  Unknown,
};

// Result of a file operation
struct IOResult {
  IOErrorType error;
  std::string message;
  size_t bytes_processed;
  int retry_count;
  bool recovered;

  bool IsSuccess() const { return error == IOErrorType::Success; }
  bool RequiresRetry() const {
    return error == IOErrorType::ReadError ||
           error == IOErrorType::DeviceNotReady ||
           error == IOErrorType::InterferenceDetected ||
           error == IOErrorType::Timeout ||
           error == IOErrorType::PartialRead;
  }
};

// Configuration for robust file operations
struct RobustIOConfig {
  // Retry configuration
  int max_retries = 5;
  int retry_delay_ms = 100;
  bool exponential_backoff = true;

  // Verification
  bool verify_checksum = true;
  bool verify_file_size = true;

  // Performance
  size_t read_chunk_size = 1024 * 1024;  // 1MB chunks
  size_t buffer_size = 4096 * 1024;       // 4MB buffer

  // Interference detection
  bool detect_interference = true;
  int interference_threshold_ms = 500;  // Slow reads suggest interference

  // Error handling
  bool fail_fast = false;  // Stop on first error if true
  bool log_errors = true;
};

// Robust file reader with retry logic and error recovery
class RobustFileReader {
 public:
  explicit RobustFileReader(const RobustIOConfig& config = RobustIOConfig());
  ~RobustFileReader();

  // Read entire file with retry logic
  IOResult ReadFile(const std::filesystem::path& path,
                    std::vector<uint8_t>& data);

  // Read file in chunks with progress callback
  IOResult ReadFileChunked(
      const std::filesystem::path& path, std::vector<uint8_t>& data,
      std::function<void(size_t bytes_read, size_t total_size)> progress_cb =
          nullptr);

  // Read file with checksum verification
  IOResult ReadFileVerified(const std::filesystem::path& path,
                            std::vector<uint8_t>& data, uint32_t expected_crc);

  // Check if file is accessible and ready
  IOResult VerifyFileAccess(const std::filesystem::path& path);

  // Get statistics
  int GetTotalRetries() const { return total_retries_; }
  int GetInterferenceDetections() const { return interference_count_; }
  int GetRecoveredErrors() const { return recovered_errors_; }

 private:
  IOResult ReadWithRetry(const std::filesystem::path& path,
                         std::vector<uint8_t>& data);
  IOResult VerifyData(const std::vector<uint8_t>& data, size_t expected_size);
  uint32_t CalculateCRC32(const std::vector<uint8_t>& data);
  bool DetectInterference(uint64_t read_time_ms, size_t bytes_read);
  void WaitBeforeRetry(int retry_count);

  RobustIOConfig config_;
  int total_retries_;
  int interference_count_;
  int recovered_errors_;
};

// Robust file writer with verification
class RobustFileWriter {
 public:
  explicit RobustFileWriter(const RobustIOConfig& config = RobustIOConfig());
  ~RobustFileWriter();

  // Write file with retry logic
  IOResult WriteFile(const std::filesystem::path& path,
                     const std::vector<uint8_t>& data);

  // Write file with verification
  IOResult WriteFileVerified(const std::filesystem::path& path,
                             const std::vector<uint8_t>& data);

  // Atomic write (write to temp, then rename)
  IOResult WriteFileAtomic(const std::filesystem::path& path,
                           const std::vector<uint8_t>& data);

 private:
  IOResult WriteWithRetry(const std::filesystem::path& path,
                          const std::vector<uint8_t>& data);
  IOResult VerifyWrite(const std::filesystem::path& path,
                       const std::vector<uint8_t>& original_data);

  RobustIOConfig config_;
  int total_retries_;
};

// Interference detector and mitigator
class InterferenceDetector {
 public:
  static InterferenceDetector& GetInstance();

  // Detect current interference level
  enum class InterferenceLevel {
    None,
    Low,      // Slight slowdowns
    Medium,   // Noticeable delays
    High,     // Severe interference
    Critical, // System barely responsive
  };

  InterferenceLevel DetectCurrentLevel();

  // Record an I/O timing sample
  void RecordIOTiming(uint64_t duration_ms, size_t bytes);

  // Check if interference is currently active
  bool IsInterferenceActive() const;

  // Get recommended mitigation strategy
  std::string GetMitigationAdvice() const;

  // Statistics
  uint64_t GetAverageIOTime() const { return avg_io_time_ms_; }
  size_t GetInterferenceCount() const { return interference_count_; }

 private:
  InterferenceDetector();
  ~InterferenceDetector();

  InterferenceDetector(const InterferenceDetector&) = delete;
  InterferenceDetector& operator=(const InterferenceDetector&) = delete;

  struct IOSample {
    uint64_t timestamp;
    uint64_t duration_ms;
    size_t bytes;
  };

  std::vector<IOSample> recent_samples_;
  uint64_t avg_io_time_ms_;
  size_t interference_count_;
  InterferenceLevel current_level_;
};

// Helper functions for common operations
namespace helpers {

// Load a game file with full error recovery
IOResult LoadGameFile(const std::filesystem::path& path,
                      std::vector<uint8_t>& data);

// Load a game file with progress reporting
IOResult LoadGameFileWithProgress(
    const std::filesystem::path& path, std::vector<uint8_t>& data,
    std::function<void(int progress_percent)> progress_cb);

// Check if a file is corrupted
bool IsFileCorrupted(const std::filesystem::path& path);

// Repair a corrupted file if possible
IOResult RepairFile(const std::filesystem::path& path);

}  // namespace helpers

}  // namespace robust_io
}  // namespace xe

#endif  // XENIA_BASE_ROBUST_FILE_IO_H_
