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

dword_result_t XamVoiceIsActiveProcess_entry() {
  // Returning 0 here will short-circuit a bunch of voice stuff.
  return 0;
}
DECLARE_XAM_EXPORT1(XamVoiceIsActiveProcess, kNone, kStub);

dword_result_t XamVoiceCreate_entry(unknown_t unk1,  // 0
                                    unknown_t unk2,  // 0xF
                                    lpdword_t out_voice_ptr) {
  // Null out the ptr.
  out_voice_ptr.Zero();
  return X_ERROR_ACCESS_DENIED;
}
DECLARE_XAM_EXPORT1(XamVoiceCreate, kNone, kStub);

dword_result_t XamVoiceClose_entry(lpunknown_t voice_ptr) { return 0; }
DECLARE_XAM_EXPORT1(XamVoiceClose, kNone, kStub);

dword_result_t XamVoiceHeadsetPresent_entry(lpunknown_t voice_ptr) { return 0; }
DECLARE_XAM_EXPORT1(XamVoiceHeadsetPresent, kNone, kStub);

dword_result_t XamVoiceSetMicArrayIdleUsers_entry(dword_t user_mask) {
  // XamVoiceSetMicArrayIdleUsers configures which users are idle for the
  // Kinect microphone array. This is used in games with voice chat or
  // Kinect support to manage which users should be monitored by the mic array.
  // For emulation, we stub this as success since we don't have real Kinect
  // hardware or voice chat functionality.
  XELOGD("XamVoiceSetMicArrayIdleUsers({:08X}) - stubbed", user_mask);
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamVoiceSetMicArrayIdleUsers, kNone, kStub);

}  // namespace xam
}  // namespace kernel
}  // namespace xe

DECLARE_XAM_EMPTY_REGISTER_EXPORTS(Voice);
