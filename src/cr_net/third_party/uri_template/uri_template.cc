/*
 * \copyright Copyright 2013 Google Inc. All Rights Reserved.
 * \license @{
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @}
 */

// Implementation of RFC 6570 based on (open source implementation) at
//   java/com/google/api/client/http/UriTemplate.java
// The URI Template spec is at http://tools.ietf.org/html/rfc6570
// Templates up to level 3 are supported.

#include "cr_net/third_party/uri_template/uri_template.h"

#include <set>
#include <string>
#include <vector>

#include "cr_base/strings/string_split.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/string_number_conversions.h"

using std::string;

namespace uri_template {

namespace {

const char kHexString[] = "0123456789ABCDEF";
inline char IntToHex(int i) {
  CR_DCHECK(i >= 0) << i << " not a hex value";
  CR_DCHECK(i <= 15) << i << " not a hex value";
  return kHexString[i];
}

// A fast bit-vector map for ascii characters.
//
// Internally stores 256 bits in an array of 8 ints.
// Does quick bit-flicking to lookup needed characters.
struct Charmap {
  bool Contains(unsigned char c) const {
    return ((map[c >> 5] & (1 << (c & 31))) != 0);
  }

  uint32_t map[8];
};

// Given text to escape and a Charmap defining which values to escape,
// return an escaped string.  If use_plus is true, spaces are converted
// to +, otherwise, if spaces are in the charmap, they are converted to
// %20. And if keep_escaped is true, %XX will be kept as it is, otherwise, if
// '%' is in the charmap, it is converted to %25.
std::string Escape(cr::StringPiece text,
                   const Charmap& charmap,
                   bool use_plus,
                   bool keep_escaped = false) {
  std::string escaped;
  escaped.reserve(text.length() * 3);
  for (unsigned int i = 0; i < text.length(); ++i) {
    unsigned char c = static_cast<unsigned char>(text[i]);
    if (use_plus && ' ' == c) {
      escaped.push_back('+');
    } else if (keep_escaped && '%' == c && i + 2 < text.length() &&
               cr::IsAsciiHexDigit(text[i + 1]) && 
               cr::IsAsciiHexDigit(text[i + 2])) {
      escaped.push_back('%');
    } else if (charmap.Contains(c)) {
      escaped.push_back('%');
      escaped.push_back(IntToHex(c >> 4));
      escaped.push_back(IntToHex(c & 0xf));
    } else {
      escaped.push_back(c);
    }
  }
  return escaped;
}

// Everything except alphanumerics and -._~
// See RFC 3986 for the list of unreserved characters.
static const Charmap kUnreservedCharmap = {
    {0xffffffffL, 0xfc009fffL, 0x78000001L, 0xb8000001L, 0xffffffffL,
     0xffffffffL, 0xffffffffL, 0xffffffffL}};

// The UriTemplateConfig is used to represent variable sections and to construct
// the expanded url.
struct UriTemplateConfig {
 public:
  UriTemplateConfig(const char* prefix,
                    const char* joiner,
                    bool requires_variable_assignment,
                    bool allow_reserved_expansion,
                    bool no_variable_assignment_if_empty = false)
      : prefix_(prefix),
        joiner_(joiner),
        requires_variable_assignment_(requires_variable_assignment),
        no_variable_assignment_if_empty_(no_variable_assignment_if_empty),
        allow_reserved_expansion_(allow_reserved_expansion) {}

  void AppendValue(const string& variable,
                   const string& value,
                   bool use_prefix,
                   string* target) const {
    string joiner = use_prefix ? prefix_ : joiner_;
    if (requires_variable_assignment_) {
      if (value.empty() && no_variable_assignment_if_empty_) {
        target->append(joiner + EscapedValue(variable));
      } else {
        target->append(joiner + EscapedValue(variable) + "=" +
                       EscapedValue(value));
      }
    } else {
      target->append(joiner + EscapedValue(value));
    }
  }

 private:
  string EscapedValue(const string& value) const {
    string escaped;
    if (allow_reserved_expansion_) {
      // Reserved expansion passes through reserved and pct-encoded characters.
      escaped = Escape(value, kUnreservedCharmap, false);
    } else {
      escaped = Escape(value, kUnreservedCharmap, false);
    }
    return escaped;
  }

  const char* prefix_;
  const char* joiner_;
  bool requires_variable_assignment_;
  bool no_variable_assignment_if_empty_;
  bool allow_reserved_expansion_;
};

// variable is an in-out argument. On input it is the content between the
// '{}' in the source. On result the control parameters are stripped off
// leaving just the comma-separated variable name(s) that we should try to
// resolve.
UriTemplateConfig MakeConfig(string* variable) {
  switch (*variable->data()) {
    // Reserved expansion.
    case '+':
      *variable = variable->substr(1);
      return UriTemplateConfig("", ",", false, true);

    // Fragment expansion.
    case '#':
      *variable = variable->substr(1);
      return UriTemplateConfig("#", ",", false, true);

    // Label with dot-prefix.
    case '.':
      *variable = variable->substr(1);
      return UriTemplateConfig(".", ".", false, false);

    // Path segment expansion.
    case '/':
      *variable = variable->substr(1);
      return UriTemplateConfig("/", "/", false, false);

    // Path segment parameter expansion.
    case ';':
      *variable = variable->substr(1);
      return UriTemplateConfig(";", ";", true, false, true);

    // Form-style query expansion.
    case '?':
      *variable = variable->substr(1);
      return UriTemplateConfig("?", "&", true, false);

    // Form-style query continuation.
    case '&':
      *variable = variable->substr(1);
      return UriTemplateConfig("&", "&", true, false);

    // Simple expansion.
    default:
      return UriTemplateConfig("", ",", false, false);
  }
}

void ProcessVariableSection(
    string* variable_section,
    const std::unordered_map<string, string>& parameters,
    string* target,
    std::set<string>* vars_found) {
  // Note that this function will modify the variable_section string to remove
  // the decorators, leaving just comma-separated variable name(s).
  UriTemplateConfig config = MakeConfig(variable_section);
  std::vector<string> variables = cr::SplitString(
      *variable_section, ",", cr::KEEP_WHITESPACE, cr::SPLIT_WANT_NONEMPTY);
  bool first_var = true;
  for (const string& variable : variables) {
    auto found = parameters.find(variable);
    if (found != parameters.end()) {
      config.AppendValue(variable, found->second, first_var, target);
      first_var = false;
      if (vars_found) {
        vars_found->insert(variable);
      }
    }
  }
}

}  // namespace

bool Expand(const string& path_uri,
            const std::unordered_map<string, string>& parameters,
            string* target,
            std::set<string>* vars_found) {
  size_t cur = 0;
  size_t length = path_uri.length();
  while (cur < length) {
    size_t open = path_uri.find('{', cur);
    size_t close = path_uri.find('}', cur);
    if (open == string::npos) {
      if (close == string::npos) {
        // No more variables to process.
        target->append(path_uri.substr(cur).data(), path_uri.length() - cur);
        return true;
      } else {
        // Template was malformed. Unexpected closing brace.
        target->clear();
        return false;
      }
    }
    target->append(path_uri, cur, open - cur);
    size_t next_open = path_uri.find('{', open + 1);
    if (close == string::npos || close < open || next_open < close) {
      // Template was malformed.
      target->clear();
      return false;
    }
    string variable_section(path_uri, open + 1, close - open - 1);
    cur = close + 1;

    ProcessVariableSection(&variable_section, parameters, target, vars_found);
  }
  return true;
}

}  // namespace uri_template