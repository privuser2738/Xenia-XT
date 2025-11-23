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
#include "xenia/kernel/xboxkrnl/xboxkrnl_private.h"
#include "xenia/xbox.h"

namespace xe {
namespace kernel {
namespace xboxkrnl {

void HalReturnToFirmware_entry(dword_t routine) {
  // void
  // IN FIRMWARE_REENTRY  Routine
  //
  // Routine values:
  // 0 = HalHaltRoutine - halt the system
  // 1 = HalRebootRoutine - reboot  
  // 2 = HalKdRebootRoutine - reboot into kernel debugger
  // 3 = HalFatalErrorRebootRoutine - reboot due to fatal error
  // 4 = HalPowerDownRoutine - power off
  // 5 = HalRebootQuiesceRoutine - quiet reboot
  // 6 = HalForceShutdownRoutine - force shutdown
  
  XELOGI("HalReturnToFirmware called with routine {}", (uint32_t)routine);
  
  switch (routine) {
    case 0:  // HalHaltRoutine
      XELOGI("Game requested halt");
      break;
    case 1:  // HalRebootRoutine  
      XELOGI("Game requested reboot/exit");
      break;
    case 4:  // HalPowerDownRoutine
      XELOGI("Game requested power down");
      break;
    default:
      XELOGI("Game requested firmware return (routine {})", (uint32_t)routine);
      break;
  }
  
  // Request graceful termination through the kernel
  kernel_state()->TerminateTitle();
}
DECLARE_XBOXKRNL_EXPORT2(HalReturnToFirmware, kNone, kImplemented, kImportant);

}  // namespace xboxkrnl
}  // namespace kernel
}  // namespace xe

DECLARE_XBOXKRNL_EMPTY_REGISTER_EXPORTS(Hal);
