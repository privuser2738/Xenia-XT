/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2025. All rights reserved.                                       *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_KERNEL_UTIL_XEX_BC_ANALYZER_H_
#define XENIA_KERNEL_UTIL_XEX_BC_ANALYZER_H_

#include <string>
#include <vector>

#include "xenia/kernel/util/xex2_info.h"

namespace xe {
namespace kernel {

// Backwards Compatibility Analyzer for Xbox One BC
// Analyzes XEX headers to provide compatibility hints based on
// features and flags commonly seen in well-behaved games that
// run well on Xbox One backwards compatibility.
class XexBCAnalyzer {
 public:
  struct BCHint {
    enum class Level {
      kGood,     // Feature that indicates good compatibility
      kNeutral,  // Feature with no impact
      kConcern   // Feature that may cause compatibility issues
    };

    Level level;
    std::string message;
  };

  // Analyze XEX system flags for BC compatibility hints
  static std::vector<BCHint> AnalyzeSystemFlags(uint32_t system_flags);

  // Analyze XEX image flags for BC compatibility hints
  static std::vector<BCHint> AnalyzeImageFlags(uint32_t image_flags);

  // Analyze XEX media flags for BC compatibility hints
  static std::vector<BCHint> AnalyzeMediaFlags(uint32_t media_flags);

  // Get overall BC compatibility assessment
  static std::string GetCompatibilityAssessment(
      uint32_t system_flags, uint32_t image_flags, uint32_t media_flags);
};

}  // namespace kernel
}  // namespace xe

#endif  // XENIA_KERNEL_UTIL_XEX_BC_ANALYZER_H_
