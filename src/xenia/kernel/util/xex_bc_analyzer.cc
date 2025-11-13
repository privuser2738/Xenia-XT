/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025. All rights reserved.                                       *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/util/xex_bc_analyzer.h"

#include "xenia/base/logging.h"

namespace xe {
namespace kernel {

std::vector<XexBCAnalyzer::BCHint> XexBCAnalyzer::AnalyzeSystemFlags(
    uint32_t system_flags) {
  std::vector<BCHint> hints;

  // Good compatibility indicators
  if (system_flags & XEX_SYSTEM_NO_FORCED_REBOOT) {
    hints.push_back({BCHint::Level::kGood,
                     "No forced reboot - good for suspend/resume"});
  }

  if (system_flags & XEX_SYSTEM_ALLOW_BACKGROUND_DOWNLOAD) {
    hints.push_back({BCHint::Level::kGood,
                     "Allows background downloads - well-behaved networking"});
  }

  if (system_flags & XEX_SYSTEM_ALLOW_CONTROLLER_SWAPPING) {
    hints.push_back(
        {BCHint::Level::kGood, "Allows controller swapping - good UX"});
  }

  if (system_flags & XEX_SYSTEM_MULTIDISC_SWAP) {
    hints.push_back(
        {BCHint::Level::kNeutral, "Supports multi-disc swap functionality"});
  }

  if (system_flags & XEX_SYSTEM_MULTIDISC_CROSS_TITLE) {
    hints.push_back({BCHint::Level::kNeutral,
                     "Supports cross-title multi-disc (like Mass Effect)"});
  }

  // Potential concerns for BC
  if (system_flags & XEX_SYSTEM_INSECURE_SOCKETS) {
    hints.push_back({BCHint::Level::kConcern,
                     "Uses insecure sockets - may need network emulation"});
  }

  if (system_flags & XEX_SYSTEM_INSECURE_UTILITY_DRIVE) {
    hints.push_back({BCHint::Level::kConcern,
                     "Uses insecure utility drive - may need special handling"});
  }

  if (system_flags & XEX_SYSTEM_NO_ODD_MAPPING) {
    hints.push_back({BCHint::Level::kConcern,
                     "No ODD mapping - requires different disc handling"});
  }

  if (system_flags & XEX_SYSTEM_XBOX1_INTEROPERABILITY) {
    hints.push_back(
        {BCHint::Level::kConcern,
         "Xbox 1 interoperability - may have legacy compatibility code"});
  }

  return hints;
}

std::vector<XexBCAnalyzer::BCHint> XexBCAnalyzer::AnalyzeImageFlags(
    uint32_t image_flags) {
  std::vector<BCHint> hints;

  if (image_flags & XEX_IMAGE_REGION_FREE) {
    hints.push_back(
        {BCHint::Level::kGood, "Region-free - works on all consoles"});
  }

  if (image_flags & XEX_IMAGE_PAGE_SIZE_4KB) {
    hints.push_back({BCHint::Level::kGood,
                     "4KB page size - compatible with modern systems"});
  } else {
    hints.push_back({BCHint::Level::kNeutral,
                     "64KB page size - standard Xbox 360 configuration"});
  }

  if (image_flags & XEX_IMAGE_XGD2_MEDIA_ONLY) {
    hints.push_back(
        {BCHint::Level::kNeutral, "XGD2 media only - disc-based game"});
  }

  if (image_flags & XEX_IMAGE_REVOCATION_CHECK_REQUIRED) {
    hints.push_back({BCHint::Level::kConcern,
                     "Requires revocation check - online validation needed"});
  }

  if (image_flags & XEX_IMAGE_ONLINE_ACTIVATION_REQUIRED) {
    hints.push_back({BCHint::Level::kConcern,
                     "Requires online activation - DRM restrictions"});
  }

  return hints;
}

std::vector<XexBCAnalyzer::BCHint> XexBCAnalyzer::AnalyzeMediaFlags(
    uint32_t media_flags) {
  std::vector<BCHint> hints;

  if (media_flags & XEX_MEDIA_HARDDISK) {
    hints.push_back(
        {BCHint::Level::kGood, "Supports hard disk - can be installed"});
  }

  if (media_flags & XEX_MEDIA_NETWORK) {
    hints.push_back({BCHint::Level::kNeutral,
                     "Supports network - may have online features"});
  }

  if (media_flags & XEX_MEDIA_SVOD) {
    hints.push_back({BCHint::Level::kGood,
                     "Supports SVOD - System Video on Demand (disc install)"});
  }

  if (media_flags & (XEX_MEDIA_DVD_X2 | XEX_MEDIA_DVD_5 | XEX_MEDIA_DVD_9)) {
    hints.push_back({BCHint::Level::kNeutral, "DVD-based game"});
  }

  if (media_flags & XEX_MEDIA_INSECURE_PACKAGE) {
    hints.push_back({BCHint::Level::kConcern,
                     "Uses insecure packages - development/debug build"});
  }

  return hints;
}

std::string XexBCAnalyzer::GetCompatibilityAssessment(uint32_t system_flags,
                                                      uint32_t image_flags,
                                                      uint32_t media_flags) {
  auto system_hints = AnalyzeSystemFlags(system_flags);
  auto image_hints = AnalyzeImageFlags(image_flags);
  auto media_hints = AnalyzeMediaFlags(media_flags);

  int good_count = 0;
  int concern_count = 0;

  for (const auto& hint : system_hints) {
    if (hint.level == BCHint::Level::kGood)
      good_count++;
    else if (hint.level == BCHint::Level::kConcern)
      concern_count++;
  }
  for (const auto& hint : image_hints) {
    if (hint.level == BCHint::Level::kGood)
      good_count++;
    else if (hint.level == BCHint::Level::kConcern)
      concern_count++;
  }
  for (const auto& hint : media_hints) {
    if (hint.level == BCHint::Level::kGood)
      good_count++;
    else if (hint.level == BCHint::Level::kConcern)
      concern_count++;
  }

  if (concern_count == 0 && good_count >= 3) {
    return "Excellent - Well-behaved game with good BC compatibility "
           "indicators";
  } else if (concern_count <= 1 && good_count >= 2) {
    return "Good - Should work well on Xbox One BC";
  } else if (concern_count <= 2) {
    return "Fair - May work on Xbox One BC with minor issues";
  } else {
    return "Challenging - Has features that may complicate BC emulation";
  }
}

}  // namespace kernel
}  // namespace xe
