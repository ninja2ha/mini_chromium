// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/i18n/case_conversion.h"

#include "cr_base/logging/logging.h"
#include "cr_base/memory/no_destructor.h"

namespace cr {
namespace i18n {

namespace {

// 0x0100 - 0x0217
int32_t QueryLower_0100_0x217(int32_t c) {
  static constexpr int32_t kMin = 0x0100;
  static constexpr int32_t kMax = 0x0217;
  static uint16_t s_table_v[kMax - kMin + 1];

  static cr::NoDestructor<uint16_t*> s_table([](){
    memset(s_table_v, 0, sizeof(s_table_v));

    for (int32_t i = 0x0100; i <= 0x0136; i += 2) {
      s_table_v[i - kMin] = i + 1;
    }
    s_table_v[0x0130 - kMin] = 0x0069;

    for (int32_t i = 0x0139; i <= 0x0147; i += 2) {
      s_table_v[i - kMin] = i + 1;
    }

    for (int32_t i = 0x014A; i <= 0x0176; i += 2) {
      s_table_v[i - kMin] = i + 1;
    }

    s_table_v[0x0178 - kMin] = 0x00FF;
    for (int32_t i = 0x0179; i <= 0x017D; i +=2 ) {
      s_table_v[i - kMin] = i + 1;
    }

    s_table_v[0x0181 - kMin] = 0x0253;
    s_table_v[0x0182 - kMin] = 0x0183;
    s_table_v[0x0184 - kMin] = 0x0185;
    s_table_v[0x0186 - kMin] = 0x0254;
    s_table_v[0x0187 - kMin] = 0x0188;
    s_table_v[0x018A - kMin] = 0x0257;
    s_table_v[0x018B - kMin] = 0x018C;
    s_table_v[0x018E - kMin] = 0x0258;
    s_table_v[0x018F - kMin] = 0x0259;
    s_table_v[0x0190 - kMin] = 0x025B;
    s_table_v[0x0191 - kMin] = 0x0192;
    s_table_v[0x0193 - kMin] = 0x0260;
    s_table_v[0x0194 - kMin] = 0x0263;
    s_table_v[0x0196 - kMin] = 0x0269;
    s_table_v[0x0197 - kMin] = 0x0268;
    s_table_v[0x0198 - kMin] = 0x0199;
    s_table_v[0x019C - kMin] = 0x026F;
    s_table_v[0x019D - kMin] = 0x0272;
    s_table_v[0x019F - kMin] = 0x0275;

    for (int32_t i = 0x1A0; i <= 0x1A4; i += 2) {
      s_table_v[i - kMin] = i + 1;
    }
    s_table_v[0x01A7 - kMin] = 0x01A8;
    s_table_v[0x01A9 - kMin] = 0x0283;
    s_table_v[0x01AC - kMin] = 0x01AD;
    s_table_v[0x01AE - kMin] = 0x0288;
    s_table_v[0x01AF - kMin] = 0x01B0;
    s_table_v[0x01B1 - kMin] = 0x028A;
    s_table_v[0x01B2 - kMin] = 0x028B;
    s_table_v[0x01B3 - kMin] = 0x01B4;
    s_table_v[0x01B5 - kMin] = 0x01B6;
    s_table_v[0x01B7 - kMin] = 0x0292;
    s_table_v[0x01B8 - kMin] = 0x01B9;
    s_table_v[0x01BC - kMin] = 0x01BD;

    s_table_v[0x01C4 - kMin] = 0x01C6;
    s_table_v[0x01C5 - kMin] = 0x01C6;
    s_table_v[0x01C7 - kMin] = 0x01C9;
    s_table_v[0x01C8 - kMin] = 0x01C9;
    s_table_v[0x01CA - kMin] = 0x01CC;
    s_table_v[0x01CB - kMin] = 0x01CC;

    for (int32_t i = 0x1CD; i <= 0x1EE; i += 2) {
      s_table_v[i - kMin] = i + 1;
    }

    s_table_v[0x01F1 - kMin] = 0x01F3;
    s_table_v[0x01F4 - kMin] = 0x01F5;

    for (int32_t i = 0x1FA; i <= 0x0216; i += 2) {
      s_table_v[i - kMin] = i + 1;
    }
    return s_table_v;
  }());

  if ( c < kMin || c > kMax)
    return c;

  int32_t r = (*s_table)[c - kMin];
  return r == 0 ? c : r;
}

inline bool IsInRange(int32_t c, int32_t min, int32_t max) {
  return c >= min && c <= max;
}

}  // namespace

// refer from :https://www.ibm.com/docs/zh-tw/i/7.6.0?topic=tables-unicode-uppercase-lowercase-conversion-mapping-table
int32_t ToLower(int32_t c) {
  CR_DCHECK(c >= 0);

  if (IsInRange(c, 0x0041, 0x005A)) return c + 0x20;
  if (IsInRange(c, 0x00C0, 0x00DE)) return c + 0x20;

  int32_t high = c >> 8;
  if (high >= 0xFF) goto label_FF; // > 0xFF00
  if (high >= 0x20) goto label_20; // > 0x2000
  if (high >= 0x1E) goto label_1E; // > 0x1E00
  if (high >= 0x10) goto label_10; // > 0x0F00
  if (high >= 0x06) goto label_06; // > 0x0600
  if (high >= 0x04) goto label_04; // > 0x0300
  if (high == 0x03) goto label_03; // 
  return QueryLower_0100_0x217(c);

label_03:
  if (c == 0x0386) return 0x03AC;
  if (c == 0x0388) return 0x03AD;
  if (c == 0x0389) return 0x03AE;
  if (c == 0x038A) return 0x03AF;
  if (c == 0x038C) return 0x03CC;
  if (c == 0x038E) return 0x03CD;
  if (c == 0x038F) return 0x03CE;
  if (c >= 0x0391 && c <= 0x03AB) return (c == 0x03A2) ? c : c + 0x20;
  if (c >= 0x03E2 && c <= 0x03EE) return (c & 1) == 0 ? c + 1 : c;
  return c;

label_04:
  if (c >= 0x0401 && c <= 0x040F) return c + 0x50;
  if (c >= 0x0410 && c <= 0x042F) return c + 0x20;
  if (c >= 0x0460 && c <= 0x0480) return (c & 1) == 0 ? c + 1 : c;
  if (c >= 0x0490 && c <= 0x04BE) return (c & 1) == 0 ? c + 1 : c;
  if (c >= 0x04C1 && c <= 0x04F8) return (c == 0x04C5 || c == 0x04C9 || 
                                          c == 0x04CD || c == 0x04CF) ? c :
                                             (c & 1) == 1 ? c : c + 1;
  if (c >= 0x0531 && c <= 0x0556) return c + 0x30;
label_06:
  return c;

label_10:
  if (c >= 0x10A0 && c <= 0x10C5) return c + 0x30;
  return c;

label_1E:
  if (c >= 0x1E00 && c <= 0x1E94) return (c & 1) == 1 ? c : c + 1;
  if (c >= 0x1EA0 && c <= 0x1EF8) return (c & 1) == 1 ? c : c + 1;
  if (c >= 0x1F08 && c <= 0x1F0F) return c - 8;
  if (c >= 0x1F18 && c <= 0x1F1D) return c - 8;
  if (c >= 0x1F28 && c <= 0x1F2F) return c - 8;
  if (c >= 0x1F48 && c <= 0x1F4D) return c - 8;
  if (c >= 0x1F59 && c <= 0x1F5F) return c - 8;
  if (c >= 0x1F68 && c <= 0x1F6F) return c - 8;
  if (c >= 0x1F88 && c <= 0x1F8F) return c - 8;
  if (c >= 0x1F98 && c <= 0x1F9F) return c - 8;
  if (c >= 0x1FA8 && c <= 0x1FAF) return c - 8;
  if (c == 0x1FB8 || c == 0x1FB9) return c - 8;
  if (c == 0x1FD8 || c == 0x1FD9) return c - 8;
  if (c == 0x1FE8 || c == 0x1FE9) return c - 8;
  return c;

label_20:
  if (c >= 0x24B6 && c <= 0x24CF) return c + 0x1A;
  return c;

label_FF:
  if (c >= 0xFF21 && c <= 0xFF3A) return c + 0x20;
  return c;
}

}  // namespace i18n
}  // namespace cr