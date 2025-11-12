/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2020 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/vfs/devices/disc_image_device.h"

#include <chrono>
#include <thread>

#include "xenia/base/literals.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/robust_file_io.h"
#include "xenia/vfs/devices/disc_image_entry.h"

namespace xe {
namespace vfs {

using namespace xe::literals;

const size_t kXESectorSize = 2_KiB;

DiscImageDevice::DiscImageDevice(const std::string_view mount_path,
                                 const std::filesystem::path& host_path)
    : Device(mount_path), name_("GDFX"), host_path_(host_path) {}

DiscImageDevice::~DiscImageDevice() = default;

bool DiscImageDevice::Initialize() {
  XELOGI("=== Loading Disc Image with Robust I/O ===");
  XELOGI("  Path: {}", xe::path_to_utf8(host_path_));

  // Check for interference before loading
  auto& interference = robust_io::InterferenceDetector::GetInstance();
  if (interference.IsInterferenceActive()) {
    XELOGW("WARNING: Interference detected before loading!");
    XELOGW("  {}", interference.GetMitigationAdvice());
    XELOGW("  Continuing with retry logic enabled...");
  }

  // Attempt to open with retry logic
  const int max_retries = 5;
  for (int retry = 0; retry <= max_retries; retry++) {
    if (retry > 0) {
      XELOGW("Retry attempt {} of {} for disc image", retry, max_retries);

      // Wait before retry (exponential backoff)
      int wait_ms = 100 * (1 << (retry - 1));
      wait_ms = std::min(wait_ms, 2000);  // Max 2 seconds
      XELOGI("  Waiting {}ms before retry...", wait_ms);
      std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));

      // Check interference level
      if (interference.IsInterferenceActive()) {
        XELOGW("  Interference still detected: {}",
               interference.GetMitigationAdvice());
      }
    }

    auto start_time = std::chrono::steady_clock::now();
    mmap_ = MappedMemory::Open(host_path_, MappedMemory::Mode::kRead);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start_time)
                        .count();

    // Record timing for interference detection
    if (mmap_) {
      interference.RecordIOTiming(duration, mmap_->size());
    }

    if (!mmap_) {
      XELOGE("Disc image could not be mapped (attempt {})", retry + 1);

      // Check if file exists
      if (!std::filesystem::exists(host_path_)) {
        XELOGE("  File does not exist!");
        return false;  // No point retrying if file doesn't exist
      }

      // Check if it's a device/access issue
      if (retry == max_retries) {
        XELOGE("  All retry attempts exhausted");
        XELOGE("  Try:");
        XELOGE("    1. Moving phone away from PC");
        XELOGE("    2. Disabling Bluetooth/WiFi temporarily");
        XELOGE("    3. Using a different USB port or drive");
        return false;
      }

      continue;  // Retry
    }

    // Successfully mapped!
    if (retry > 0) {
      XELOGI("Successfully mapped disc image after {} retries", retry);
    }
    XELOGI("  File size: {} MB", mmap_->size() / (1024 * 1024));
    XELOGI("  Load time: {}ms", duration);

    // Check if load was slow (possible interference)
    size_t size_mb = mmap_->size() / (1024 * 1024);
    if (size_mb > 0 && duration > static_cast<int64_t>(size_mb * 20)) {  // More than 20ms per MB
      XELOGW("  Load was slower than expected - possible interference");
      XELOGW("  Consider moving phone away or disabling wireless");
    }

    break;  // Success!
  }

  if (!mmap_) {
    XELOGE("Failed to map disc image after all retries");
    return false;
  }

  ParseState state = {0};
  state.ptr = mmap_->data();
  state.size = mmap_->size();

  XELOGI("  Verifying disc image...");
  auto result = Verify(&state);
  if (result != Error::kSuccess) {
    XELOGE("Failed to verify disc image header: {}", static_cast<int>(result));

    // Check for corruption
    if (result == Error::kErrorDamagedFile) {
      XELOGE("  Disc image appears to be corrupted!");
      XELOGE("  This could be caused by:");
      XELOGE("    - Interference during previous copy");
      XELOGE("    - Bad USB connection");
      XELOGE("    - Faulty storage device");
      XELOGE("  Try re-copying the disc image");
    }

    return false;
  }

  XELOGI("  Reading directory entries...");
  result = ReadAllEntries(&state, state.ptr + state.root_offset);
  if (result != Error::kSuccess) {
    XELOGE("Failed to read all GDFX entries: {}", static_cast<int>(result));
    return false;
  }

  XELOGI("=== Disc Image Loaded Successfully ===");
  return true;
}

void DiscImageDevice::Dump(StringBuffer* string_buffer) {
  auto global_lock = global_critical_region_.Acquire();
  root_entry_->Dump(string_buffer, 0);
}

Entry* DiscImageDevice::ResolvePath(const std::string_view path) {
  // The filesystem will have stripped our prefix off already, so the path will
  // be in the form:
  // some\PATH.foo
  XELOGFS("DiscImageDevice::ResolvePath({})", path);
  return root_entry_->ResolvePath(path);
}

DiscImageDevice::Error DiscImageDevice::Verify(ParseState* state) {
  // Find sector 32 of the game partition - try at a few points.
  static const size_t likely_offsets[] = {
      0x00000000, 0x0000FB20, 0x00020600, 0x02080000, 0x0FD90000,
  };
  bool magic_found = false;
  XELOGI("  Searching for GDFX magic at {} known offsets...", xe::countof(likely_offsets));
  for (size_t n = 0; n < xe::countof(likely_offsets); n++) {
    state->game_offset = likely_offsets[n];
    size_t magic_offset = state->game_offset + (32 * kXESectorSize);
    if (VerifyMagic(state, magic_offset)) {
      magic_found = true;
      XELOGI("  Found GDFX magic at game_offset 0x{:X} (sector 32 at 0x{:X})",
             state->game_offset, magic_offset);
      break;
    }
  }
  if (!magic_found) {
    // File doesn't have the magic values - likely not a real GDFX source.
    XELOGE("  GDFX magic not found - this is not a valid Xbox 360 disc image");
    return Error::kErrorFileMismatch;
  }

  // Read sector 32 to get FS state.
  if (state->size < state->game_offset + (32 * kXESectorSize)) {
    XELOGE("  File too small for GDFX header");
    return Error::kErrorReadError;
  }
  uint8_t* fs_ptr = state->ptr + state->game_offset + (32 * kXESectorSize);
  state->root_sector = xe::load<uint32_t>(fs_ptr + 20);
  state->root_size = xe::load<uint32_t>(fs_ptr + 24);
  state->root_offset =
      state->game_offset + (state->root_sector * kXESectorSize);

  XELOGI("  Root directory info:");
  XELOGI("    Root sector: {} (0x{:X})", state->root_sector, state->root_sector);
  XELOGI("    Root size: {} bytes (0x{:X})", state->root_size, state->root_size);
  XELOGI("    Root offset: 0x{:X}", state->root_offset);
  XELOGI("    File size: 0x{:X} ({} MB)", state->size, state->size / (1024 * 1024));

  if (state->root_size < 13 || state->root_size > 32_MiB) {
    XELOGE("  Invalid root size: {} bytes (expected 13 to {} bytes)",
           state->root_size, 32_MiB);
    return Error::kErrorDamagedFile;
  }

  // Validate root offset is within file
  if (state->root_offset >= state->size) {
    XELOGE("  Root offset 0x{:X} exceeds file size 0x{:X}",
           state->root_offset, state->size);
    return Error::kErrorDamagedFile;
  }

  if (state->root_offset + state->root_size > state->size) {
    XELOGE("  Root directory (0x{:X} + 0x{:X}) exceeds file size 0x{:X}",
           state->root_offset, state->root_size, state->size);
    return Error::kErrorDamagedFile;
  }

  XELOGI("  GDFX header validated successfully");
  return Error::kSuccess;
}

bool DiscImageDevice::VerifyMagic(ParseState* state, size_t offset) {
  if (offset >= state->size) {
    return false;
  }

  // Simple check to see if the given offset contains the magic value.
  return std::memcmp(state->ptr + offset, "MICROSOFT*XBOX*MEDIA", 20) == 0;
}

DiscImageDevice::Error DiscImageDevice::ReadAllEntries(
    ParseState* state, const uint8_t* root_buffer) {
  auto root_entry = new DiscImageEntry(this, nullptr, "", mmap_.get());
  root_entry->attributes_ = kFileAttributeDirectory;
  root_entry_ = std::unique_ptr<Entry>(root_entry);

  XELOGI("  Parsing root directory:");
  XELOGI("    Root buffer offset: 0x{:X}", root_buffer - state->ptr);
  XELOGI("    Root buffer size: {} bytes", state->root_size);
  XELOGI("    First 16 bytes: {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}",
         root_buffer[0], root_buffer[1], root_buffer[2], root_buffer[3],
         root_buffer[4], root_buffer[5], root_buffer[6], root_buffer[7],
         root_buffer[8], root_buffer[9], root_buffer[10], root_buffer[11],
         root_buffer[12], root_buffer[13], root_buffer[14], root_buffer[15]);

  if (!ReadEntry(state, root_buffer, 0, root_entry)) {
    XELOGW("  WARNING: ReadEntry had errors, but some entries may have been loaded");
    // Don't return error - check if we got any entries
  }

  // Count what we loaded and list files
  size_t total_entries = 0;
  size_t total_files = 0;
  size_t total_dirs = 0;
  std::vector<std::string> file_list;
  std::function<void(Entry*, std::string)> count_entries = [&](Entry* entry, std::string path) {
    total_entries++;
    std::string full_path = path + "/" + entry->name();
    if (entry->attributes() & kFileAttributeDirectory) {
      total_dirs++;
      for (auto& child : entry->children()) {
        count_entries(child.get(), full_path);
      }
    } else {
      total_files++;
      file_list.push_back(full_path);
    }
  };

  for (auto& child : root_entry->children_) {
    count_entries(child.get(), "");
  }

  XELOGI("=== Disc Image Load Summary ===");
  XELOGI("  Root entries: {}", root_entry->children_.size());
  XELOGI("  Total entries loaded: {}", total_entries);
  XELOGI("  Files: {}", total_files);
  XELOGI("  Directories: {}", total_dirs);

  if (total_files > 0) {
    XELOGI("  Accessible files:");
    for (const auto& file : file_list) {
      XELOGI("    {}", file);
    }
  }

  if (total_entries == 0) {
    XELOGE("  FAILED: No entries could be loaded from this disc image");
    return Error::kErrorOutOfMemory;
  }

  XELOGI("  SUCCESS: Loaded {} accessible entries", total_entries);
  return Error::kSuccess;
}

bool DiscImageDevice::ReadEntry(ParseState* state, const uint8_t* buffer,
                                uint16_t entry_ordinal,
                                DiscImageEntry* parent, size_t buffer_size,
                                int depth,
                                std::unordered_set<uint16_t>* visited) {
  // Security: Create visited set on first call
  std::unordered_set<uint16_t> local_visited;
  if (!visited) {
    visited = &local_visited;
    // Use root_size as buffer_size if not specified
    if (buffer_size == 0) {
      buffer_size = state->root_size;
    }
  }

  // Security: Check recursion depth limit
  constexpr int kMaxRecursionDepth = 256;
  if (depth > kMaxRecursionDepth) {
    XELOGE(
        "Disc image: Recursion depth limit ({}) exceeded, possible corrupted "
        "or malicious file",
        kMaxRecursionDepth);
    return false;
  }

  // Security: Check for circular references
  if (visited->count(entry_ordinal)) {
    XELOGE(
        "Disc image: Circular entry reference detected at ordinal {}, "
        "possible corrupted or malicious file",
        entry_ordinal);
    return false;
  }
  visited->insert(entry_ordinal);

  // Security: Bounds check for entry_ordinal
  // Each entry is 4 bytes for header, and minimum 14 bytes of data
  constexpr size_t kMinEntrySize = 4 + 14;
  size_t entry_offset = static_cast<size_t>(entry_ordinal) * 4;
  if (buffer_size > 0 && entry_offset + kMinEntrySize > buffer_size) {
    XELOGE(
        "Disc image: Entry ordinal {} out of bounds (offset {} + {} > {}), "
        "possible corrupted or malicious file",
        entry_ordinal, entry_offset, kMinEntrySize, buffer_size);
    XELOGE("  Depth: {}", depth);
    return false;
  }

  if (depth <= 2) {
    XELOGI("  Reading entry {} at offset {} (depth {})", entry_ordinal, entry_offset, depth);
  }

  const uint8_t* p = buffer + entry_offset;

  uint16_t node_l = xe::load<uint16_t>(p + 0);
  uint16_t node_r = xe::load<uint16_t>(p + 2);
  size_t sector = xe::load<uint32_t>(p + 4);
  size_t length = xe::load<uint32_t>(p + 8);
  uint8_t attributes = xe::load<uint8_t>(p + 12);
  uint8_t name_length = xe::load<uint8_t>(p + 13);
  auto name_buffer = reinterpret_cast<const char*>(p + 14);

  // Log entry details at depth 0 (root level)
  if (depth == 0) {
    XELOGI("    Entry {} at offset {}: node_l={}, node_r={}, sector={}, length={}, attr=0x{:02X}, name_len={}",
           entry_ordinal, entry_offset, node_l, node_r, sector, length, attributes, name_length);
  }

  // Security: Validate name_length doesn't exceed bounds
  if (buffer_size > 0 && entry_offset + 14 + name_length > buffer_size) {
    XELOGE(
        "Disc image: Entry ordinal {} name length {} exceeds buffer bounds (offset {} + 14 + {} > {}), "
        "possible corrupted or malicious file",
        entry_ordinal, name_length, entry_offset, name_length, buffer_size);
    XELOGE("    Entry details: node_l={}, node_r={}, sector={}, length={}, attr=0x{:02X}",
           node_l, node_r, sector, length, attributes);
    return false;
  }

  if (node_l) {
    if (!ReadEntry(state, buffer, node_l, parent, buffer_size, depth + 1,
                   visited)) {
      XELOGW("  WARNING: Failed to read left child (node_l={}) of entry {} at depth {}",
             node_l, entry_ordinal, depth);
      XELOGW("    Skipping this subtree and continuing...");
      // Don't return false - continue processing this entry and its right sibling
    }
  }

  auto name = std::string(name_buffer, name_length);

  auto entry = DiscImageEntry::Create(this, parent, name, mmap_.get());
  entry->attributes_ = attributes | kFileAttributeReadOnly;
  entry->size_ = length;
  entry->allocation_size_ = xe::round_up(length, bytes_per_sector());

  // Set to January 1, 1970 (UTC) in 100-nanosecond intervals
  entry->create_timestamp_ = 10000 * 11644473600000LL;
  entry->access_timestamp_ = 10000 * 11644473600000LL;
  entry->write_timestamp_ = 10000 * 11644473600000LL;

  if (attributes & kFileAttributeDirectory) {
    // Folder.
    entry->data_offset_ = 0;
    entry->data_size_ = 0;
    if (length) {
      // Not a leaf - read in children.
      size_t folder_offset = state->game_offset + (sector * kXESectorSize);
      if (state->size < folder_offset) {
        // Out of bounds read - directory data is beyond file size
        XELOGW("  WARNING: Directory '{}' sector {} at offset 0x{:X} exceeds file size 0x{:X}",
               name, sector, folder_offset, state->size);
        XELOGW("    This directory is INACCESSIBLE (truncated/corrupt ISO)");
        XELOGW("    Continuing to load other directories...");
        // Don't fail - just mark this directory as empty and continue
        entry->size_ = 0;
      } else {
        // Read child list.
        if (depth <= 2) {
          XELOGI("  Directory '{}': reading children from sector {} (offset 0x{:X}, length {})",
                 name, sector, folder_offset, length);
        }
        uint8_t* folder_ptr = state->ptr + folder_offset;
        // New buffer for subfolder, so reset visited set but keep depth tracking
        if (!ReadEntry(state, folder_ptr, 0, entry.get(), length, depth + 1,
                       nullptr)) {
          XELOGW("  WARNING: Failed to read children of directory '{}'", name);
          XELOGW("    Directory may be corrupt or have invalid entries");
          XELOGW("    Continuing to load other directories...");
          // Don't fail - just leave this directory empty and continue
        }
      }
    }
  } else {
    // File.
    size_t file_offset = state->game_offset + (sector * kXESectorSize);
    if (file_offset >= state->size) {
      XELOGW("  WARNING: File '{}' sector {} at offset 0x{:X} exceeds file size 0x{:X}",
             name, sector, file_offset, state->size);
      XELOGW("    This file is INACCESSIBLE (truncated/corrupt ISO)");
      // Mark file as invalid
      entry->data_offset_ = 0;
      entry->data_size_ = 0;
      entry->size_ = 0;
    } else {
      entry->data_offset_ = file_offset;
      entry->data_size_ = length;
    }
  }

  // Add to parent.
  parent->children_.emplace_back(std::move(entry));

  // Read next file in the list.
  if (node_r) {
    if (!ReadEntry(state, buffer, node_r, parent, buffer_size, depth + 1,
                   visited)) {
      XELOGW("  WARNING: Failed to read right child (node_r={}) of entry {} at depth {}",
             node_r, entry_ordinal, depth);
      XELOGW("    Skipping this subtree and continuing...");
      // Don't return false - we've added the current entry, just skip this sibling
    }
  }

  return true;
}

}  // namespace vfs
}  // namespace xe
