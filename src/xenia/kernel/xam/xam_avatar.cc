/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/logging.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/util/shim_utils.h"
#include "xenia/kernel/xam/xam_private.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
namespace xam {

// Avatar system is not implemented - we return failure which tells games
// that avatars are unavailable. Most games handle this gracefully.

dword_result_t XamAvatarInitialize_entry(
    dword_t unk1,              // 1, 4, etc
    dword_t unk2,              // 0 or 1
    dword_t processor_number,  // for thread creation?
    lpdword_t function_ptrs,   // 20b, 5 pointers
    lpunknown_t unk5,          // ptr in data segment
    dword_t unk6               // flags - 0x00300000, 0x30, etc
) {
  XELOGD("XamAvatarInitialize - avatars not supported, returning failure");
  // Return negative value to indicate avatars are not available.
  // Games should handle this gracefully and call XamAvatarShutdown.
  return 0x80004005;  // E_FAIL - more standard error code
}
DECLARE_XAM_EXPORT1(XamAvatarInitialize, kAvatars, kImplemented);

void XamAvatarShutdown_entry() {
  XELOGD("XamAvatarShutdown");
  // No-op - nothing to clean up since we didn't initialize anything.
}
DECLARE_XAM_EXPORT1(XamAvatarShutdown, kAvatars, kImplemented);

}  // namespace xam
}  // namespace kernel
}  // namespace xe

DECLARE_XAM_EMPTY_REGISTER_EXPORTS(Avatar);
