/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/xam/xam_content_device.h"

#include "xenia/base/cvar.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xam/xam_private.h"
#include "xenia/kernel/xenumerator.h"
#include "xenia/xbox.h"

DEFINE_bool(hdd_disabled, false,
            "Disable the virtual HDD device (some games require HDD to launch).",
            "Storage");

DEFINE_uint64(hdd_total_size, 40,
              "Total size of virtual HDD in GB (Xbox 360 options: 20, 60, 120, 250, 320, 500).",
              "Storage");

DEFINE_uint64(hdd_free_size, 0,
              "Free space on virtual HDD in GB (0 = auto, leaves ~10% used).",
              "Storage");

namespace xe {
namespace kernel {
namespace xam {

#define ONE_GB (1024ull * 1024ull * 1024ull)

// Dynamic HDD device info that can be configured via CVARs
static DummyDeviceInfo GetDynamicHddDeviceInfo() {
  static bool logged_config = false;

  uint64_t total_size = cvars::hdd_total_size * ONE_GB;
  uint64_t free_size;

  if (cvars::hdd_free_size > 0) {
    free_size = cvars::hdd_free_size * ONE_GB;
  } else {
    // Auto: make it look ~10% used (90% free)
    free_size = (total_size * 9) / 10;
  }

  // Ensure free doesn't exceed total
  if (free_size > total_size) {
    free_size = total_size;
  }

  if (!logged_config) {
    XELOGI("Virtual HDD: Enabled - Total: {}GB, Free: {}GB",
           cvars::hdd_total_size,
           free_size / ONE_GB);
    logged_config = true;
  }

  return {DummyDeviceId::HDD, DeviceType::HDD, total_size, free_size, u"Xenia Virtual HDD"};
}

static const DummyDeviceInfo dummy_hdd_device_info_ = {
    DummyDeviceId::HDD, DeviceType::HDD,
    20ull * ONE_GB,  // 20GB (will be overridden by dynamic function)
    3ull * ONE_GB,   // 3GB (will be overridden by dynamic function)
    u"Dummy HDD",
};
static const DummyDeviceInfo dummy_odd_device_info_ = {
    DummyDeviceId::ODD, DeviceType::ODD,
    7ull * ONE_GB,  // 7GB (rough maximum)
    0ull * ONE_GB,  // read-only FS, so no free space
    u"Dummy ODD",
};
static const DummyDeviceInfo* dummy_device_infos_[] = {
    &dummy_hdd_device_info_,
    &dummy_odd_device_info_,
};
#undef ONE_GB

const DummyDeviceInfo* GetDummyDeviceInfo(uint32_t device_id) {
  // Special handling for HDD to use dynamic configuration
  if (device_id == static_cast<uint32_t>(DummyDeviceId::HDD)) {
    // If HDD is disabled, return nullptr to indicate device not connected
    if (cvars::hdd_disabled) {
      static bool logged_disabled = false;
      if (!logged_disabled) {
        XELOGI("Virtual HDD: Disabled (hdd_disabled = true)");
        logged_disabled = true;
      }
      return nullptr;
    }

    // Use static storage to avoid assignment issues with const members
    static std::u16string hdd_name;
    auto hdd_info = GetDynamicHddDeviceInfo();
    hdd_name = std::u16string(hdd_info.name);

    static DummyDeviceInfo dynamic_hdd = {
        DummyDeviceId::HDD, DeviceType::HDD, 0, 0, hdd_name
    };

    // Manually update mutable fields (can't use assignment due to const member)
    const_cast<uint64_t&>(dynamic_hdd.total_bytes) = hdd_info.total_bytes;
    const_cast<uint64_t&>(dynamic_hdd.free_bytes) = hdd_info.free_bytes;
    const_cast<std::u16string_view&>(dynamic_hdd.name) = hdd_name;

    return &dynamic_hdd;
  }

  // For other devices, use the static lookup
  const auto& begin = std::begin(dummy_device_infos_);
  const auto& end = std::end(dummy_device_infos_);
  auto it = std::find_if(begin, end, [device_id](const auto& item) {
    return static_cast<uint32_t>(item->device_id) == device_id;
  });
  return it == end ? nullptr : *it;
}

dword_result_t XamContentGetDeviceName_entry(dword_t device_id,
                                             lpu16string_t name_buffer,
                                             dword_t name_capacity) {
  auto device_info = GetDummyDeviceInfo(device_id);
  if (device_info == nullptr) {
    return X_ERROR_DEVICE_NOT_CONNECTED;
  }
  auto name = std::u16string(device_info->name);
  if (name_capacity < name.size() + 1) {
    return X_ERROR_INSUFFICIENT_BUFFER;
  }
  xe::string_util::copy_and_swap_truncating(name_buffer, name, name_capacity);
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamContentGetDeviceName, kContent, kImplemented);

dword_result_t XamContentGetDeviceState_entry(dword_t device_id,
                                              lpunknown_t overlapped_ptr) {
  auto device_info = GetDummyDeviceInfo(device_id);
  if (device_info == nullptr) {
    if (overlapped_ptr) {
      kernel_state()->CompleteOverlappedImmediateEx(
          overlapped_ptr, X_ERROR_FUNCTION_FAILED, X_ERROR_DEVICE_NOT_CONNECTED,
          0);
      return X_ERROR_IO_PENDING;
    } else {
      return X_ERROR_DEVICE_NOT_CONNECTED;
    }
  }
  if (overlapped_ptr) {
    kernel_state()->CompleteOverlappedImmediate(overlapped_ptr,
                                                X_ERROR_SUCCESS);
    return X_ERROR_IO_PENDING;
  } else {
    return X_ERROR_SUCCESS;
  }
}
DECLARE_XAM_EXPORT1(XamContentGetDeviceState, kContent, kStub);

typedef struct {
  xe::be<uint32_t> device_id;
  xe::be<uint32_t> device_type;
  xe::be<uint64_t> total_bytes;
  xe::be<uint64_t> free_bytes;
  union {
    xe::be<uint16_t> name[28];
    char16_t name_chars[28];
  };
} X_CONTENT_DEVICE_DATA;
static_assert_size(X_CONTENT_DEVICE_DATA, 0x50);

dword_result_t XamContentGetDeviceData_entry(
    dword_t device_id, pointer_t<X_CONTENT_DEVICE_DATA> device_data) {
  auto device_info = GetDummyDeviceInfo(device_id);
  if (device_info == nullptr) {
    return X_ERROR_DEVICE_NOT_CONNECTED;
  }
  device_data.Zero();
  device_data->device_id = static_cast<uint32_t>(device_info->device_id);
  device_data->device_type = static_cast<uint32_t>(device_info->device_type);
  device_data->total_bytes = device_info->total_bytes;
  device_data->free_bytes = device_info->free_bytes;
  xe::string_util::copy_and_swap_truncating(
      device_data->name_chars, device_info->name,
      xe::countof(device_data->name_chars));
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamContentGetDeviceData, kContent, kImplemented);

dword_result_t XamContentCreateDeviceEnumerator_entry(dword_t content_type,
                                                      dword_t content_flags,
                                                      dword_t max_count,
                                                      lpdword_t buffer_size_ptr,
                                                      lpdword_t handle_out) {
  assert_not_null(handle_out);

  if (buffer_size_ptr) {
    *buffer_size_ptr = sizeof(X_CONTENT_DEVICE_DATA) * max_count;
  }

  auto e = make_object<XStaticEnumerator<X_CONTENT_DEVICE_DATA>>(kernel_state(),
                                                                 max_count);
  auto result = e->Initialize(0xFE, 0xFE, 0x2000A, 0x20009, 0);
  if (XFAILED(result)) {
    return result;
  }

  // Add HDD device (using dynamic info)
  if (!cvars::hdd_disabled) {
    auto hdd_info = GetDynamicHddDeviceInfo();
    auto device_data = e->AppendItem();
    assert_not_null(device_data);
    if (device_data) {
      device_data->device_id = static_cast<uint32_t>(hdd_info.device_id);
      device_data->device_type = static_cast<uint32_t>(hdd_info.device_type);
      device_data->total_bytes = hdd_info.total_bytes;
      device_data->free_bytes = hdd_info.free_bytes;
      xe::string_util::copy_and_swap_truncating(
          device_data->name_chars, hdd_info.name,
          xe::countof(device_data->name_chars));
    }
  }

  // Add ODD device (optical disc drive)
  auto device_data = e->AppendItem();
  assert_not_null(device_data);
  if (device_data) {
    device_data->device_id = static_cast<uint32_t>(dummy_odd_device_info_.device_id);
    device_data->device_type = static_cast<uint32_t>(dummy_odd_device_info_.device_type);
    device_data->total_bytes = dummy_odd_device_info_.total_bytes;
    device_data->free_bytes = dummy_odd_device_info_.free_bytes;
    xe::string_util::copy_and_swap_truncating(
        device_data->name_chars, dummy_odd_device_info_.name,
        xe::countof(device_data->name_chars));
  }

  *handle_out = e->handle();
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamContentCreateDeviceEnumerator, kNone, kImplemented);

}  // namespace xam
}  // namespace kernel
}  // namespace xe

DECLARE_XAM_EMPTY_REGISTER_EXPORTS(ContentDevice);
