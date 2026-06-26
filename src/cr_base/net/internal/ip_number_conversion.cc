// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/net/internal/ip_number_conversion.h"

#include <string.h>

#include <string>

#include "cr_base/logging/logging.h"
#include "cr_base/strings/string_util.h"

namespace cr {
namespace net {

namespace internal {

namespace {

struct Component {
  Component() : begin(-1), len(0) {};
  Component(int a, int b) : begin(a), len(b) {};
  bool is_valid() const { return begin != -1;};
  int end() const { return begin + len;}
  void reset() {begin = -1; len = 0;};
  bool is_nonempty() const {return begin != -1 && len != 0;};

  int begin;
  int len;
};

// Bits that identify different character types. These types identify different
// bits that are set for each 8-bit character in the kSharedCharTypeTable.
enum SharedCharTypes {
  // Characters that do not require escaping in queries. Characters that do
  // not have this flag will be escaped; see url_canon_query.cc
  CHAR_QUERY = 1,

  // Valid in the username/password field.
  CHAR_USERINFO = 2,

  // Valid in a IPv4 address (digits plus dot and 'x' for hex).
  CHAR_IPV4 = 4,

  // Valid in an ASCII-representation of a hex digit (as in %-escaped).
  CHAR_HEX = 8,

  // Valid in an ASCII-representation of a decimal digit.
  CHAR_DEC = 16,

  // Valid in an ASCII-representation of an octal digit.
  CHAR_OCT = 32,

  // Characters that do not require escaping in encodeURIComponent. Characters
  // that do not have this flag will be escaped; see url_util.cc.
  CHAR_COMPONENT = 64,
};


// See the header file for this array's declaration.
const unsigned char kSharedCharTypeTable[0x100] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x00 - 0x0f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x10 - 0x1f
    0,                           // 0x20  ' ' (escape spaces in queries)
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x21  !
    0,                           // 0x22  "
    0,                           // 0x23  #  (invalid in query since it marks the ref)
    CHAR_QUERY | CHAR_USERINFO,  // 0x24  $
    CHAR_QUERY | CHAR_USERINFO,  // 0x25  %
    CHAR_QUERY | CHAR_USERINFO,  // 0x26  &
    0,                           // 0x27  '  (Try to prevent XSS.)
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x28  (
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x29  )
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x2a  *
    CHAR_QUERY | CHAR_USERINFO,  // 0x2b  +
    CHAR_QUERY | CHAR_USERINFO,  // 0x2c  ,
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x2d  -
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_COMPONENT,  // 0x2e  .
    CHAR_QUERY,                  // 0x2f  /
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x30  0
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x31  1
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x32  2
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x33  3
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x34  4
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x35  5
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x36  6
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_OCT | CHAR_COMPONENT,  // 0x37  7
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_COMPONENT,             // 0x38  8
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_DEC | CHAR_COMPONENT,             // 0x39  9
    CHAR_QUERY,  // 0x3a  :
    CHAR_QUERY,  // 0x3b  ;
    0,           // 0x3c  <  (Try to prevent certain types of XSS.)
    CHAR_QUERY,  // 0x3d  =
    0,           // 0x3e  >  (Try to prevent certain types of XSS.)
    CHAR_QUERY,  // 0x3f  ?
    CHAR_QUERY,  // 0x40  @
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x41  A
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x42  B
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x43  C
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x44  D
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x45  E
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x46  F
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x47  G
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x48  H
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x49  I
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x4a  J
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x4b  K
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x4c  L
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x4d  M
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x4e  N
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x4f  O
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x50  P
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x51  Q
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x52  R
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x53  S
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x54  T
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x55  U
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x56  V
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x57  W
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_COMPONENT, // 0x58  X
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x59  Y
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x5a  Z
    CHAR_QUERY,  // 0x5b  [
    CHAR_QUERY,  // 0x5c  '\'
    CHAR_QUERY,  // 0x5d  ]
    CHAR_QUERY,  // 0x5e  ^
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x5f  _
    CHAR_QUERY,  // 0x60  `
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x61  a
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x62  b
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x63  c
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x64  d
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x65  e
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_HEX | CHAR_COMPONENT,  // 0x66  f
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x67  g
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x68  h
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x69  i
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x6a  j
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x6b  k
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x6c  l
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x6d  m
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x6e  n
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x6f  o
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x70  p
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x71  q
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x72  r
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x73  s
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x74  t
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x75  u
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x76  v
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x77  w
    CHAR_QUERY | CHAR_USERINFO | CHAR_IPV4 | CHAR_COMPONENT,  // 0x78  x
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x79  y
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x7a  z
    CHAR_QUERY,  // 0x7b  {
    CHAR_QUERY,  // 0x7c  |
    CHAR_QUERY,  // 0x7d  }
    CHAR_QUERY | CHAR_USERINFO | CHAR_COMPONENT,  // 0x7e  ~
    0,           // 0x7f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x80 - 0x8f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x90 - 0x9f
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xa0 - 0xaf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xb0 - 0xbf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xc0 - 0xcf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xd0 - 0xdf
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xe0 - 0xef
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0xf0 - 0xff
};


// More readable wrappers around the character type lookup table.
inline bool IsCharOfType(unsigned char c, SharedCharTypes type) {
  return !!(kSharedCharTypeTable[c] & type);
}
inline bool IsQueryChar(unsigned char c) {
  return IsCharOfType(c, CHAR_QUERY);
}
inline bool IsIPv4Char(unsigned char c) {
  return IsCharOfType(c, CHAR_IPV4);
}
inline bool IsHexChar(unsigned char c) {
  return IsCharOfType(c, CHAR_HEX);
}
inline bool IsComponentChar(unsigned char c) {
  return IsCharOfType(c, CHAR_COMPONENT);
}


// Converts one of the character types that represent a numerical base to the
// corresponding base.
int BaseForType(SharedCharTypes type) {
  switch (type) {
    case CHAR_HEX:
      return 16;
    case CHAR_DEC:
      return 10;
    case CHAR_OCT:
      return 8;
    default:
      return 0;
  }
}

// -- ipv4 ---------------------------------------------------------------------

bool DoFindIPv4Components(const char* spec,
                          const Component& host,
                          Component components[4]) {
  if (!host.is_nonempty())
    return false;

  int cur_component = 0;  // Index of the component we're working on.
  int cur_component_begin = host.begin;  // Start of the current component.
  int end = host.end();
  for (int i = host.begin; /* nothing */; i++) {
    if (i >= end || spec[i] == '.') {
      // Found the end of the current component.
      int component_len = i - cur_component_begin;
      components[cur_component] = Component(cur_component_begin, component_len);

      // The next component starts after the dot.
      cur_component_begin = i + 1;
      cur_component++;

      // Don't allow empty components (two dots in a row), except we may
      // allow an empty component at the end (this would indicate that the
      // input ends in a dot). We also want to error if the component is
      // empty and it's the only component (cur_component == 1).
      if (component_len == 0 && (i < end || cur_component == 1))
        return false;

      if (i >= end)
        break;  // End of the input.

      if (cur_component == 4) {
        // Anything else after the 4th component is an error unless it is a
        // dot that would otherwise be treated as the end of input.
        if (spec[i] == '.' && i + 1 == end)
          break;
        return false;
      }
    } else if (static_cast<uint8_t>(spec[i]) >= 0x80 ||
               !internal::IsIPv4Char(static_cast<unsigned char>(spec[i]))) {
      // Invalid character for an IPv4 address.
      return false;
    }
  }

  // Fill in any unused components.
  while (cur_component < 4)
    components[cur_component++] = Component();
  return true;
}

// Converts an IPv4 component to a 32-bit number, while checking for overflow.
//
// The input is assumed to be ASCII. FindIPv4Components should have stripped
// out any input that is greater than 7 bits. The components are assumed
// to be non-empty.
bool IPv4ComponentToNumber(const char* spec,
                           const Component& component,
                           uint32_t* number) {
  // Figure out the base
  SharedCharTypes base;
  int base_prefix_len = 0;  // Size of the prefix for this base.
  if (spec[component.begin] == '0') {
    // Either hex or dec, or a standalone zero.
    if (component.len == 1) {
      base = CHAR_DEC;
    } else if (spec[component.begin + 1] == 'X' ||
               spec[component.begin + 1] == 'x') {
      base = CHAR_HEX;
      base_prefix_len = 2;
    } else {
      base = CHAR_OCT;
      base_prefix_len = 1;
    }
  } else {
    base = CHAR_DEC;
  }

  // Extend the prefix to consume all leading zeros.
  while (base_prefix_len < component.len &&
         spec[component.begin + base_prefix_len] == '0')
    base_prefix_len++;

  // Put the component, minus any base prefix, into a NULL-terminated buffer so
  // we can call the standard library. Because leading zeros have already been
  // discarded, filling the entire buffer is guaranteed to trigger the 32-bit
  // overflow check.
  const int kMaxComponentLen = 16;
  char buf[kMaxComponentLen + 1];  // digits + '\0'
  int dest_i = 0;
  for (int i = component.begin + base_prefix_len; i < component.end(); i++) {
    // We know the input is 7-bit, so convert to narrow (if this is the wide
    // version of the template) by casting.
    char input = static_cast<char>(spec[i]);

    // Validate that this character is OK for the given base.
    if (!IsCharOfType(input, base))
      return false;

    // Fill the buffer, if there's space remaining. This check allows us to
    // verify that all characters are numeric, even those that don't fit.
    if (dest_i < kMaxComponentLen)
      buf[dest_i++] = input;
  }

  buf[dest_i] = '\0';

  // Use the 64-bit strtoi so we get a big number (no hex, decimal, or octal
  // number can overflow a 64-bit number in <= 16 characters).
  uint64_t num = _strtoui64(buf, NULL, BaseForType(base));

  // Check for 32-bit overflow.
  if (num > std::numeric_limits<uint32_t>::max()) {
    return false;
  }

  // No overflow. Success!
  *number = static_cast<uint32_t>(num);
  return true;
}


// See declaration of IPv4AddressToNumber for documentation.
bool DoIPv4AddressToNumber(const char* spec,
                           const Component& host,
                           unsigned char address[4],
                           int* num_ipv4_components) {
  // The identified components. Not all may exist.
  Component components[4];
  if (!DoFindIPv4Components(spec, host, components)) {
    // CanonHostInfo::NEUTRAL
    return false;
  }

  // Convert existing components to digits. Values up to
  // |existing_components| will be valid.
  uint32_t component_values[4];
  int existing_components = 0;

  // Set to true if one or more components are BROKEN. BROKEN is only
  // returned if all components are IPV4 or BROKEN, so, for example,
  // 12345678912345.de returns NEUTRAL rather than broken.
  for (int i = 0; i < 4; i++) {
    if (components[i].len <= 0)
      continue;

    bool br = IPv4ComponentToNumber(
        spec, components[i], &component_values[existing_components]);
    if (!br)
      return false;

    existing_components++;
  }

  // Use that sequence of numbers to fill out the 4-component IP address.

  // First, process all components but the last, while making sure each fits
  // within an 8-bit field.
  for (int i = 0; i < existing_components - 1; i++) {
    if (component_values[i] > std::numeric_limits<uint8_t>::max()) {
      return false;
      //return CanonHostInfo::BROKEN;
    }
    address[i] = static_cast<unsigned char>(component_values[i]);
  }

  // Next, consume the last component to fill in the remaining bytes.
  // Work around a gcc 4.9 bug. crbug.com/392872
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 9) || __GNUC__ > 4)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
  uint32_t last_value = component_values[existing_components - 1];
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 9) || __GNUC__ > 4)
#pragma GCC diagnostic pop
#endif
  for (int i = 3; i >= existing_components - 1; i--) {
    address[i] = static_cast<unsigned char>(last_value);
    last_value >>= 8;
  }

  // If the last component has residual bits, report overflow.
  if (last_value != 0) {
    return false;
  }

  // Tell the caller how many components we saw.
  *num_ipv4_components = existing_components;

  // Success!
  return true;
}

// -- ipv6 ---------------------------------------------------------------------

// Helper class that describes the main components of an IPv6 input string.
// See the following examples to understand how it breaks up an input string:
//
// [Example 1]: input = "[::aa:bb]"
//  ==> num_hex_components = 2
//  ==> hex_components[0] = Component(3,2) "aa"
//  ==> hex_components[1] = Component(6,2) "bb"
//  ==> index_of_contraction = 0
//  ==> ipv4_component = Component(0, -1)
//
// [Example 2]: input = "[1:2::3:4:5]"
//  ==> num_hex_components = 5
//  ==> hex_components[0] = Component(1,1) "1"
//  ==> hex_components[1] = Component(3,1) "2"
//  ==> hex_components[2] = Component(6,1) "3"
//  ==> hex_components[3] = Component(8,1) "4"
//  ==> hex_components[4] = Component(10,1) "5"
//  ==> index_of_contraction = 2
//  ==> ipv4_component = Component(0, -1)
//
// [Example 3]: input = "[::ffff:192.168.0.1]"
//  ==> num_hex_components = 1
//  ==> hex_components[0] = Component(3,4) "ffff"
//  ==> index_of_contraction = 0
//  ==> ipv4_component = Component(8, 11) "192.168.0.1"
//
// [Example 4]: input = "[1::]"
//  ==> num_hex_components = 1
//  ==> hex_components[0] = Component(1,1) "1"
//  ==> index_of_contraction = 1
//  ==> ipv4_component = Component(0, -1)
//
// [Example 5]: input = "[::192.168.0.1]"
//  ==> num_hex_components = 0
//  ==> index_of_contraction = 0
//  ==> ipv4_component = Component(8, 11) "192.168.0.1"
//
struct IPv6Parsed {
  // Zero-out the parse information.
  void reset() {
    num_hex_components = 0;
    index_of_contraction = -1;
    ipv4_component.reset();
  }

  // There can be up to 8 hex components (colon separated) in the literal.
  Component hex_components[8];

  // The count of hex components present. Ranges from [0,8].
  int num_hex_components;

  // The index of the hex component that the "::" contraction precedes, or
  // -1 if there is no contraction.
  int index_of_contraction;

  // The range of characters which are an IPv4 literal.
  Component ipv4_component;
};

// Parse the IPv6 input string. If parsing succeeded returns true and fills
// |parsed| with the information. If parsing failed (because the input is
// invalid) returns false.
bool DoParseIPv6(const char* spec, const Component& host, IPv6Parsed* parsed) {
  // Zero-out the info.
  parsed->reset();

  if (!host.is_nonempty())
    return false;

  // The index for start and end of address range (no brackets).
  int begin = host.begin;
  int end = host.end();

  int cur_component_begin = begin;  // Start of the current component.

  // Scan through the input, searching for hex components, "::" contractions,
  // and IPv4 components.
  for (int i = begin; /* i <= end */; i++) {
    bool is_colon = spec[i] == ':';
    bool is_contraction = is_colon && i < end - 1 && spec[i + 1] == ':';

    // We reached the end of the current component if we encounter a colon
    // (separator between hex components, or start of a contraction), or end of
    // input.
    if (is_colon || i == end) {
      int component_len = i - cur_component_begin;

      // A component should not have more than 4 hex digits.
      if (component_len > 4)
        return false;

      // Don't allow empty components.
      if (component_len == 0) {
        // The exception is when contractions appear at beginning of the
        // input or at the end of the input.
        if (!((is_contraction && i == begin) || (i == end &&
            parsed->index_of_contraction == parsed->num_hex_components)))
          return false;
      }

      // Add the hex component we just found to running list.
      if (component_len > 0) {
        // Can't have more than 8 components!
        if (parsed->num_hex_components >= 8)
          return false;

        parsed->hex_components[parsed->num_hex_components++] =
            Component(cur_component_begin, component_len);
      }
    }

    if (i == end)
      break;  // Reached the end of the input, DONE.

    // We found a "::" contraction.
    if (is_contraction) {
      // There can be at most one contraction in the literal.
      if (parsed->index_of_contraction != -1)
        return false;
      parsed->index_of_contraction = parsed->num_hex_components;
      ++i;  // Consume the colon we peeked.
    }

    if (is_colon) {
      // Colons are separators between components, keep track of where the
      // current component started (after this colon).
      cur_component_begin = i + 1;
    } else {
      if (static_cast<uint8_t>(spec[i]) >= 0x80)
        return false;  // Not ASCII.

      if (!internal::IsHexChar(static_cast<unsigned char>(spec[i]))) {
        // Regular components are hex numbers. It is also possible for
        // a component to be an IPv4 address in dotted form.
        if (internal::IsIPv4Char(static_cast<unsigned char>(spec[i]))) {
          // Since IPv4 address can only appear at the end, assume the rest
          // of the string is an IPv4 address. (We will parse this separately
          // later).
          parsed->ipv4_component =
              Component(cur_component_begin, end - cur_component_begin);
          break;
        } else {
          // The character was neither a hex digit, nor an IPv4 character.
          return false;
        }
      }
    }
  }

  return true;
}

// Converts a hex component into a number. This cannot fail since the caller has
// already verified that each character in the string was a hex digit, and
// that there were no more than 4 characters.
uint16_t IPv6HexComponentToNumber(const char* spec,
                                  const Component& component) {
  CR_DCHECK(component.len <= 4);

  // Copy the hex string into a C-string.
  char buf[5];
  for (int i = 0; i < component.len; ++i)
    buf[i] = static_cast<char>(spec[component.begin + i]);
  buf[component.len] = '\0';

  // Convert it to a number (overflow is not possible, since with 4 hex
  // characters we can at most have a 16 bit number).
  return static_cast<uint16_t>(_strtoui64(buf, NULL, 16));
}

// Verifies the parsed IPv6 information, checking that the various components
// add up to the right number of bits (hex components are 16 bits, while
// embedded IPv4 formats are 32 bits, and contractions are placeholdes for
// 16 or more bits). Returns true if sizes match up, false otherwise. On
// success writes the length of the contraction (if any) to
// |out_num_bytes_of_contraction|.
bool CheckIPv6ComponentsSize(const IPv6Parsed& parsed,
                             int* out_num_bytes_of_contraction) {
  // Each group of four hex digits contributes 16 bits.
  int num_bytes_without_contraction = parsed.num_hex_components * 2;

  // If an IPv4 address was embedded at the end, it contributes 32 bits.
  if (parsed.ipv4_component.is_valid())
    num_bytes_without_contraction += 4;

  // If there was a "::" contraction, its size is going to be:
  // MAX([16bits], [128bits] - num_bytes_without_contraction).
  int num_bytes_of_contraction = 0;
  if (parsed.index_of_contraction != -1) {
    num_bytes_of_contraction = 16 - num_bytes_without_contraction;
    if (num_bytes_of_contraction < 2)
      num_bytes_of_contraction = 2;
  }

  // Check that the numbers add up.
  if (num_bytes_without_contraction + num_bytes_of_contraction != 16)
    return false;

  *out_num_bytes_of_contraction = num_bytes_of_contraction;
  return true;
}

// Converts an IPv6 address to a 128-bit number (network byte order), returning
// true on success. False means that the input was not a valid IPv6 address.
bool DoIPv6AddressToNumber(const char* spec,
                           const Component& host,
                           unsigned char address[16]) {
  // Make sure the component is bounded by '[' and ']'.
  int end = host.end();
  if (!host.is_nonempty() || spec[host.begin] != '[' || spec[end - 1] != ']')
    return false;

  // Exclude the square brackets.
  Component ipv6_comp(host.begin + 1, host.len - 2);

  // Parse the IPv6 address -- identify where all the colon separated hex
  // components are, the "::" contraction, and the embedded IPv4 address.
  IPv6Parsed ipv6_parsed;
  if (!DoParseIPv6(spec, ipv6_comp, &ipv6_parsed))
    return false;

  // Do some basic size checks to make sure that the address doesn't
  // specify more than 128 bits or fewer than 128 bits. This also resolves
  // how may zero bytes the "::" contraction represents.
  int num_bytes_of_contraction;
  if (!CheckIPv6ComponentsSize(ipv6_parsed, &num_bytes_of_contraction))
    return false;

  int cur_index_in_address = 0;

  // Loop through each hex components, and contraction in order.
  for (int i = 0; i <= ipv6_parsed.num_hex_components; ++i) {
    // Append the contraction if it appears before this component.
    if (i == ipv6_parsed.index_of_contraction) {
      for (int j = 0; j < num_bytes_of_contraction; ++j)
        address[cur_index_in_address++] = 0;
    }
    // Append the hex component's value.
    if (i != ipv6_parsed.num_hex_components) {
      // Get the 16-bit value for this hex component.
      uint16_t number = IPv6HexComponentToNumber(
          spec, ipv6_parsed.hex_components[i]);
      // Append to |address|, in network byte order.
      address[cur_index_in_address++] = (number & 0xFF00) >> 8;
      address[cur_index_in_address++] = (number & 0x00FF);
    }
  }

  // If there was an IPv4 section, convert it into a 32-bit number and append
  // it to |address|.
  if (ipv6_parsed.ipv4_component.is_valid()) {
    // Append the 32-bit number to |address|.
    int ignored_num_ipv4_components;
    if (!DoIPv4AddressToNumber(spec,
                               ipv6_parsed.ipv4_component,
                               &address[cur_index_in_address],
                               &ignored_num_ipv4_components))
      return false;
  }

  return true;
}

// Searches for the longest sequence of zeros in |address|, and writes the
// range into |contraction_range|. The run of zeros must be at least 16 bits,
// and if there is a tie the first is chosen.
void ChooseIPv6ContractionRange(const unsigned char address[16],
                                Component* contraction_range) {
  // The longest run of zeros in |address| seen so far.
  Component max_range;

  // The current run of zeros in |address| being iterated over.
  Component cur_range;

  for (int i = 0; i < 16; i += 2) {
    // Test for 16 bits worth of zero.
    bool is_zero = (address[i] == 0 && address[i + 1] == 0);

    if (is_zero) {
      // Add the zero to the current range (or start a new one).
      if (!cur_range.is_valid())
        cur_range = Component(i, 0);
      cur_range.len += 2;
    }

    if (!is_zero || i == 14) {
      // Just completed a run of zeros. If the run is greater than 16 bits,
      // it is a candidate for the contraction.
      if (cur_range.len > 2 && cur_range.len > max_range.len) {
        max_range = cur_range;
      }
      cur_range.reset();
    }
  }
  *contraction_range = max_range;
}

}  // namesapce

void AppendIPv4Address(const unsigned char address[4], 
                       std::string* output) {
  CR_DCHECK(output);

  output->clear();
  for (int i = 0; i < 4; i++) {
    char str[16];
    _itoa_s(address[i], str, 10);

    for (int ch = 0; str[ch] != 0; ch++)
      output->push_back(str[ch]);

    if (i != 3)
      output->push_back('.');
  }
}

void AppendIPv6Address(const unsigned char address[16],
                       std::string* output) {
  // We will output the address according to the rules in:
  // http://tools.ietf.org/html/draft-kawamura-ipv6-text-representation-01#section-4

  // Start by finding where to place the "::" contraction (if any).
  Component contraction_range;
  ChooseIPv6ContractionRange(address, &contraction_range);

  for (int i = 0; i <= 14;) {
    // We check 2 bytes at a time, from bytes (0, 1) to (14, 15), inclusive.
    CR_DCHECK(i % 2 == 0);
    if (i == contraction_range.begin && contraction_range.len > 0) {
      // Jump over the contraction.
      if (i == 0)
        output->push_back(':');
      output->push_back(':');
      i = contraction_range.end();
    } else {
      // Consume the next 16 bits from |address|.
      int x = address[i] << 8 | address[i + 1];

      i += 2;

      // Stringify the 16 bit number (at most requires 4 hex digits).
      char str[5];
      _itoa_s(x, str, 16);
      for (int ch = 0; str[ch] != 0; ++ch)
        output->push_back(str[ch]);

      // Put a colon after each number, except the last.
      if (i < 16)
        output->push_back(':');
    }
  }
}

bool IPv4AddressToNumber(StringPiece ip_literal,
                         unsigned char address[4],
                         int* num_ipv4_components) {
  if (ip_literal.empty())
    return false;

  if (ip_literal.length() > 1024)
    return false;  // too long

  return DoIPv4AddressToNumber(
      ip_literal.data(), 
      Component(0, static_cast<int>(ip_literal.length())),
      address,
      num_ipv4_components);
}

// Converts an IPv6 address to a 128-bit number (network byte order), returning
// true on success. False means that the input was not a valid IPv6 address.
//
// NOTE that |host| is expected to be surrounded by square brackets.
// i.e. "[::1]" rather than "::1".
bool IPv6AddressToNumber(StringPiece ip_literal, unsigned char address[16]) {
  if (ip_literal.empty())
    return false;

  if (ip_literal.length() > 1024)
    return false;  // too long
    
  return DoIPv6AddressToNumber(
      ip_literal.data(), 
      Component(0, static_cast<int>(ip_literal.length())),
      address);
}


}  // namesapce internal

}  // namespace net
}  // namespace cr