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

// Xbox 360 Party system functions
// These functions manage the Xbox Live Party system which allows players
// to form groups for multiplayer gaming. Since we don't have Xbox Live,
// we return appropriate "not available" or "empty party" results.

// Error codes for party functions
constexpr uint32_t X_ERROR_PARTY_NOT_IN_PARTY = 0x807D0003;
constexpr uint32_t X_ERROR_PARTY_NOT_AVAILABLE = 0x807D0001;

dword_result_t XamPartyGetUserList_entry(dword_t max_users,
                                         lpdword_t user_xuids) {
  // Returns list of XUIDs in the current party
  // max_users: maximum number of users to return
  // user_xuids: pointer to array of XUIDs to fill
  //
  // Return 0x807D0003 to indicate "not in a party" which games handle gracefully
  // This prevents Soul Calibur V and similar games from crashing
  XELOGD("XamPartyGetUserList(max_users={}, user_xuids={:08X}) - not in party",
         uint32_t(max_users), user_xuids.guest_address());

  // Zero out the user list to be safe
  if (user_xuids && max_users > 0) {
    for (uint32_t i = 0; i < max_users; i++) {
      user_xuids[i] = 0;
    }
  }

  return X_ERROR_PARTY_NOT_IN_PARTY;
}
DECLARE_XAM_EXPORT1(XamPartyGetUserList, kUserProfiles, kStub);

dword_result_t XamPartySendGameInvites_entry(dword_t user_index,
                                              lpqword_t xuid_recipients,
                                              dword_t num_recipients) {
  // Sends game invites to party members
  // Since we don't have Xbox Live, just succeed silently
  XELOGD("XamPartySendGameInvites(user={}, recipients={:08X}, count={}) - no-op",
         uint32_t(user_index), xuid_recipients.guest_address(),
         uint32_t(num_recipients));

  // Return success - the game will think invites were sent
  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamPartySendGameInvites, kUserProfiles, kStub);

dword_result_t XamPartySetCustomData_entry(dword_t user_index,
                                           lpvoid_t custom_data,
                                           dword_t custom_data_size) {
  // Sets custom data visible to party members
  // We accept but ignore the data since there's no party
  XELOGD("XamPartySetCustomData(user={}, data={:08X}, size={}) - no-op",
         uint32_t(user_index), custom_data.guest_address(),
         uint32_t(custom_data_size));

  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamPartySetCustomData, kUserProfiles, kStub);

dword_result_t XamPartyGetBandwidth_entry(dword_t user_index,
                                          lpdword_t bandwidth_bps) {
  // Gets available bandwidth for voice chat
  // Return a reasonable default bandwidth
  XELOGD("XamPartyGetBandwidth(user={}, bandwidth_ptr={:08X})",
         uint32_t(user_index), bandwidth_bps.guest_address());

  if (bandwidth_bps) {
    // Return 128 kbps - a reasonable default for voice chat
    *bandwidth_bps = 128000;
  }

  return X_ERROR_SUCCESS;
}
DECLARE_XAM_EXPORT1(XamPartyGetBandwidth, kUserProfiles, kStub);

dword_result_t XamPartyCreate_entry(dword_t user_index, dword_t flags) {
  // Creates a new party
  XELOGD("XamPartyCreate(user={}, flags=0x{:08X}) - not available",
         uint32_t(user_index), uint32_t(flags));

  // Return not available - offline mode
  return X_ERROR_PARTY_NOT_AVAILABLE;
}
DECLARE_XAM_EXPORT1(XamPartyCreate, kUserProfiles, kStub);

dword_result_t XamPartyJoin_entry(dword_t user_index, qword_t party_nonce) {
  // Joins an existing party
  XELOGD("XamPartyJoin(user={}, nonce=0x{:016X}) - not available",
         uint32_t(user_index), uint64_t(party_nonce));

  return X_ERROR_PARTY_NOT_AVAILABLE;
}
DECLARE_XAM_EXPORT1(XamPartyJoin, kUserProfiles, kStub);

dword_result_t XamPartyLeave_entry(dword_t user_index) {
  // Leaves the current party
  XELOGD("XamPartyLeave(user={}) - not in party", uint32_t(user_index));

  return X_ERROR_SUCCESS;  // Already not in a party, so success
}
DECLARE_XAM_EXPORT1(XamPartyLeave, kUserProfiles, kStub);

}  // namespace xam
}  // namespace kernel
}  // namespace xe

DECLARE_XAM_EMPTY_REGISTER_EXPORTS(Party);
