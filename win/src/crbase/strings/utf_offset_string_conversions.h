// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "crbase/base_export.h"
#include "crbase/strings/string16.h"
#include "crbase/strings/string_piece.h"
#include "crbase/compiler_specific.h"

namespace cr {

// A helper class and associated data structures to adjust offsets into a
// string in response to various adjustments one might do to that string
// (e.g., eliminating a range).  For details on offsets, see the comments by
// the AdjustOffsets() function below.
class CRBASE_EXPORT OffsetAdjuster {
 public:
  struct CRBASE_EXPORT Adjustment {
    Adjustment(size_t original_offset,
               size_t original_length,
               size_t output_length);

    size_t original_offset;
    size_t original_length;
    size_t output_length;
  };
  typedef std::vector<Adjustment> Adjustments;

  // Adjusts all offsets in |offsets_for_adjustment| to reflect the adjustments
  // recorded in |adjustments|.  Adjusted offsets greater than |limit| will be
  // set to string16::npos.
  //
  // Offsets represents insertion/selection points between characters: if |src|
  // is "abcd", then 0 is before 'a', 2 is between 'b' and 'c', and 4 is at the
  // end of the string.  Valid input offsets range from 0 to |src_len|.  On
  // exit, each offset will have been modified to point at the same logical
  // position in the output string.  If an offset cannot be successfully
  // adjusted (e.g., because it points into the middle of a multibyte sequence),
  // it will be set to string16::npos.
  static void AdjustOffsets(const Adjustments& adjustments,
                            std::vector<size_t>* offsets_for_adjustment,
                            size_t limit = string16::npos);

  // Adjusts the single |offset| to reflect the adjustments recorded in
  // |adjustments|.
  static void AdjustOffset(const Adjustments& adjustments,
                           size_t* offset,
                           size_t limit = string16::npos);

  // Adjusts all offsets in |offsets_for_unadjustment| to reflect the reverse
  // of the adjustments recorded in |adjustments|.  In other words, the offsets
  // provided represent offsets into an adjusted string and the caller wants
  // to know the offsets they correspond to in the original string.  If an
  // offset cannot be successfully unadjusted (e.g., because it points into
  // the middle of a multibyte sequence), it will be set to string16::npos.
  static void UnadjustOffsets(const Adjustments& adjustments,
                              std::vector<size_t>* offsets_for_unadjustment);

  // Adjusts the single |offset| to reflect the reverse of the adjustments
  // recorded in |adjustments|.
  static void UnadjustOffset(const Adjustments& adjustments,
                             size_t* offset);

  // Combines two sequential sets of adjustments, storing the combined revised
  // adjustments in |adjustments_on_adjusted_string|.  That is, suppose a
  // string was altered in some way, with the alterations recorded as
  // adjustments in |first_adjustments|.  Then suppose the resulting string is
  // further altered, with the alterations recorded as adjustments scored in
  // |adjustments_on_adjusted_string|, with the offsets recorded in these
  // adjustments being with respect to the intermediate string.  This function
  // combines the two sets of adjustments into one, storing the result in
  // |adjustments_on_adjusted_string|, whose offsets are correct with respect
  // to the original string.
  //
  // Assumes both parameters are sorted by increasing offset.
  //
  // WARNING: Only supports |first_adjustments| that involve collapsing ranges
  // of text, not expanding ranges.
  static void MergeSequentialAdjustments(
      const Adjustments& first_adjustments,
      Adjustments* adjustments_on_adjusted_string);
};

// Like the conversions in utf_string_conversions.h, but also fills in an
// |adjustments| parameter that reflects the alterations done to the string.
// It may be NULL.
CRBASE_EXPORT bool UTF8ToUTF16WithAdjustments(
    const char* src,
    size_t src_len,
    string16* output,
    cr::OffsetAdjuster::Adjustments* adjustments);
CRBASE_EXPORT string16 UTF8ToUTF16WithAdjustments(
    const cr::StringPiece& utf8,
    cr::OffsetAdjuster::Adjustments* adjustments) CR_WARN_UNUSED_RESULT;
// As above, but instead internally examines the adjustments and applies them
// to |offsets_for_adjustment|.  Input offsets greater than the length of the
// input string will be set to string16::npos.  See comments by AdjustOffsets().
CRBASE_EXPORT string16 UTF8ToUTF16AndAdjustOffsets(
    const cr::StringPiece& utf8,
    std::vector<size_t>* offsets_for_adjustment);
CRBASE_EXPORT std::string UTF16ToUTF8AndAdjustOffsets(
    const cr::StringPiece16& utf16,
    std::vector<size_t>* offsets_for_adjustment);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_