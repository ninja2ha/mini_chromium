// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/display/display.h"

#include <algorithm>

///#include "base/command_line.h"
#include "crbase/logging.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/strings/stringprintf.h"
///#include "ui/display/display_switches.h"
#include "crui/gfx/geometry/insets.h"
#include "crui/gfx/geometry/rect_conversions.h"
#include "crui/gfx/geometry/rect_f.h"
///#include "crui/gfx/icc_profile.h"
#include "crui/gfx/geometry/transform.h"

namespace crui {
namespace display {
namespace {

// This variable tracks whether the forced device scale factor switch needs to
// be read from the command line, i.e. if it is set to -1 then the command line
// is checked.
int g_has_forced_device_scale_factor = -1;

// This variable caches the forced device scale factor value which is read off
// the command line. If the cache is invalidated by setting this variable to
// -1.0, we read the forced device scale factor again.
float g_forced_device_scale_factor = -1.0;

// An alloance error epsilon cauesd by fractional scale factor to produce
// expected DP display size.
constexpr float kDisplaySizeAllowanceEpsilon = 0.01f;

bool HasForceDeviceScaleFactorImpl() {
  ///return cr::CommandLine::ForCurrentProcess()->HasSwitch(
  ///    switches::kForceDeviceScaleFactor);
  return false;
}

float GetForcedDeviceScaleFactorImpl() {
  constexpr double scale_in_double = 1.0;
  ///if (HasForceDeviceScaleFactorImpl()) {
  ///  std::string value =
  ///      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
  ///          switches::kForceDeviceScaleFactor);
  ///  if (!base::StringToDouble(value, &scale_in_double)) {
  ///    LOG(ERROR) << "Failed to parse the default device scale factor:" << value;
  ///    scale_in_double = 1.0;
  ///  }
  ///}
  return static_cast<float>(scale_in_double);
}

int64_t internal_display_id_ = -1;

///gfx::ColorSpace ForcedColorProfileStringToColorSpace(const std::string& value) {
///  if (value == "srgb")
///    return gfx::ColorSpace::CreateSRGB();
///  if (value == "display-p3-d65")
///    return gfx::ColorSpace::CreateDisplayP3D65();
///  if (value == "scrgb-linear")
///    return gfx::ColorSpace::CreateSCRGBLinear();
///  if (value == "hdr10")
///    return gfx::ColorSpace::CreateHDR10();
///  if (value == "extended-srgb")
///    return gfx::ColorSpace::CreateExtendedSRGB();
///  if (value == "generic-rgb") {
///    return gfx::ColorSpace(gfx::ColorSpace::PrimaryID::APPLE_GENERIC_RGB,
///                           gfx::ColorSpace::TransferID::GAMMA18);
///  }
///  if (value == "color-spin-gamma24") {
///    // Run this color profile through an ICC profile. The resulting color space
///    // is slightly different from the input color space, and removing the ICC
///    // profile would require rebaselineing many layout tests.
///    gfx::ColorSpace color_space(
///        gfx::ColorSpace::PrimaryID::WIDE_GAMUT_COLOR_SPIN,
///        gfx::ColorSpace::TransferID::GAMMA24);
///    return gfx::ICCProfile::FromColorSpace(color_space).GetColorSpace();
///  }
///  LOG(ERROR) << "Invalid forced color profile: \"" << value << "\"";
///  return gfx::ColorSpace::CreateSRGB();
///}

const char* ToRotationString(display::Display::Rotation rotation) {
  switch (rotation) {
    case display::Display::ROTATE_0:
      return "0";
    case display::Display::ROTATE_90:
      return "90";
    case display::Display::ROTATE_180:
      return "180";
    case display::Display::ROTATE_270:
      return "270";
  }
  CR_NOTREACHED();
  return "unkonwn";
}

}  // namespace

bool CompareDisplayIds(int64_t id1, int64_t id2) {
  if (id1 == id2)
    return false;
  // Output index is stored in the first 8 bits. See GetDisplayIdFromEDID
  // in edid_parser.cc.
  int index_1 = id1 & 0xFF;
  int index_2 = id2 & 0xFF;
  CR_DCHECK(index_1 != index_2) << id1 << " and " << id2;
  return Display::IsInternalDisplayId(id1) ||
         (index_1 < index_2 && !Display::IsInternalDisplayId(id2));
}

// static
float Display::GetForcedDeviceScaleFactor() {
  if (g_forced_device_scale_factor < 0)
    g_forced_device_scale_factor = GetForcedDeviceScaleFactorImpl();
  return g_forced_device_scale_factor;
}

// static
bool Display::HasForceDeviceScaleFactor() {
  if (g_has_forced_device_scale_factor == -1)
    g_has_forced_device_scale_factor = HasForceDeviceScaleFactorImpl();
  return !!g_has_forced_device_scale_factor;
}

// static
void Display::ResetForceDeviceScaleFactorForTesting() {
  g_has_forced_device_scale_factor = -1;
  g_forced_device_scale_factor = -1.0;
}

// static
void Display::SetForceDeviceScaleFactor(double dsf) {
  // Reset any previously set values and unset the flag.
  g_has_forced_device_scale_factor = -1;
  g_forced_device_scale_factor = -1.0;

  ///base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(
  ///    switches::kForceDeviceScaleFactor, base::StringPrintf("%.2f", dsf));
}

// static
///gfx::ColorSpace Display::GetForcedDisplayColorProfile() {
///  DCHECK(HasForceDisplayColorProfile());
///  std::string value =
///      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
///          switches::kForceDisplayColorProfile);
///  return ForcedColorProfileStringToColorSpace(value);
///}

// static
bool Display::HasForceDisplayColorProfile() {
  ///return base::CommandLine::ForCurrentProcess()->HasSwitch(
  ///    switches::kForceDisplayColorProfile);
  return false;
}

// static
///gfx::ColorSpace Display::GetForcedRasterColorProfile() {
///  DCHECK(HasForceRasterColorProfile());
///  std::string value =
///      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
///          switches::kForceRasterColorProfile);
///  return ForcedColorProfileStringToColorSpace(value);
///}

// static
bool Display::HasForceRasterColorProfile() {
  ///return base::CommandLine::ForCurrentProcess()->HasSwitch(
  ///    switches::kForceRasterColorProfile);
  return false;
}

// static
bool Display::HasEnsureForcedColorProfile() {
  ///static bool has_ensure_forced_color_profile =
  ///    base::CommandLine::ForCurrentProcess()->HasSwitch(
  ///        switches::kEnsureForcedColorProfile);
  ///return has_ensure_forced_color_profile;
  return false;
}

// static
display::Display::Rotation Display::DegreesToRotation(int degrees) {
  if (degrees == 0)
    return display::Display::ROTATE_0;
  if (degrees == 90)
    return display::Display::ROTATE_90;
  if (degrees == 180)
    return display::Display::ROTATE_180;
  if (degrees == 270)
    return display::Display::ROTATE_270;
  CR_NOTREACHED();
  return display::Display::ROTATE_0;
}

// static
int Display::RotationToDegrees(display::Display::Rotation rotation) {
  switch (rotation) {
    case display::Display::ROTATE_0:
      return 0;
    case display::Display::ROTATE_90:
      return 90;
    case display::Display::ROTATE_180:
      return 180;
    case display::Display::ROTATE_270:
      return 270;
  }
  CR_NOTREACHED();
  return 0;
}

// static
bool Display::IsValidRotation(int degrees) {
  return degrees == 0 || degrees == 90 || degrees == 180 || degrees == 270;
}

Display::Display() : Display(kInvalidDisplayId) {}

Display::Display(int64_t id) : Display(id, gfx::Rect()) {}

Display::Display(int64_t id, const gfx::Rect& bounds)
    : id_(id),
      bounds_(bounds),
      work_area_(bounds),
      device_scale_factor_(GetForcedDeviceScaleFactor()) {
  // On Android we need to ensure the platform supports a color profile before
  // using it. Using a not supported profile can result in fatal errors in the
  // GPU process.
///  auto color_space = gfx::ColorSpace::CreateSRGB();
///#if !defined(MINI_CHROMIUM_OS_ANDROID)
///  if (HasForceDisplayColorProfile())
///    color_space = GetForcedDisplayColorProfile();
///#endif
///  SetColorSpaceAndDepth(color_space);

#if defined(MINI_CHROMIUM_USE_AURA)
  if (!bounds.IsEmpty())
    SetScaleAndBounds(device_scale_factor_, bounds);
#endif
}

Display::Display(const Display& other) = default;

Display::~Display() {}

// static
Display Display::GetDefaultDisplay() {
  return Display(kDefaultDisplayId, gfx::Rect(0, 0, 1920, 1080));
}

int Display::RotationAsDegree() const {
  switch (rotation_) {
    case ROTATE_0:
      return 0;
    case ROTATE_90:
      return 90;
    case ROTATE_180:
      return 180;
    case ROTATE_270:
      return 270;
  }
  CR_NOTREACHED();
  return 0;
}

void Display::SetRotationAsDegree(int rotation) {
  switch (rotation) {
    case 0:
      rotation_ = ROTATE_0;
      break;
    case 90:
      rotation_ = ROTATE_90;
      break;
    case 180:
      rotation_ = ROTATE_180;
      break;
    case 270:
      rotation_ = ROTATE_270;
      break;
    default:
      // We should not reach that but we will just ignore the call if we do.
      CR_NOTREACHED();
  }
}

int Display::PanelRotationAsDegree() const {
  return RotationToDegrees(panel_rotation());
}

gfx::Insets Display::GetWorkAreaInsets() const {
  return gfx::Insets(work_area_.y() - bounds_.y(), work_area_.x() - bounds_.x(),
                     bounds_.bottom() - work_area_.bottom(),
                     bounds_.right() - work_area_.right());
}

void Display::SetScaleAndBounds(float device_scale_factor,
                                const gfx::Rect& bounds_in_pixel) {
  gfx::Insets insets = bounds_.InsetsFrom(work_area_);
  if (!HasForceDeviceScaleFactor()) {
#if defined(MINI_CHROMIUM_OS_MACOSX)
    // Unless an explicit scale factor was provided for testing, ensure the
    // scale is integral.
    device_scale_factor = static_cast<int>(device_scale_factor);
#endif
    device_scale_factor_ = device_scale_factor;
  }
  device_scale_factor_ = std::max(0.5f, device_scale_factor_);

  gfx::RectF f(bounds_in_pixel);
  f.Scale(1.f / device_scale_factor_);
  bounds_ = gfx::ToEnclosedRectIgnoringError(f, kDisplaySizeAllowanceEpsilon);
  size_in_pixels_ = bounds_in_pixel.size();
  UpdateWorkAreaFromInsets(insets);
}

void Display::SetSize(const gfx::Size& size_in_pixel) {
  gfx::Point origin = bounds_.origin();
#if defined(MINI_CHROMIUM_USE_AURA)
  origin = gfx::ScaleToFlooredPoint(origin, device_scale_factor_);
#endif
  SetScaleAndBounds(device_scale_factor_, gfx::Rect(origin, size_in_pixel));
}

///gfx::ColorSpace Display::color_space() const {
///  return color_spaces_.hdr_transparent;
///}

///void Display::set_color_space(const gfx::ColorSpace& color_space) {
///  color_spaces_ = gfx::DisplayColorSpaces(color_space);
///}
///
///float Display::sdr_white_level() const {
///  return color_spaces_.sdr_white_level;
///}
///
///void Display::SetColorSpaceAndDepth(const gfx::ColorSpace& color_space,
///                                    float sdr_white_level) {
///  color_spaces_ = gfx::DisplayColorSpaces(color_space);
///  color_spaces_.sdr_white_level = sdr_white_level;
///  if (color_spaces_.SupportsHDR()) {
///    color_depth_ = kHDR10BitsPerPixel;
///    depth_per_component_ = kHDR10BitsPerComponent;
///  } else {
///    color_depth_ = kDefaultBitsPerPixel;
///    depth_per_component_ = kDefaultBitsPerComponent;
///  }
///}

void Display::UpdateWorkAreaFromInsets(const gfx::Insets& insets) {
  work_area_ = bounds_;
  work_area_.Inset(insets);
}

gfx::Size Display::GetSizeInPixel() const {
  if (!size_in_pixels_.IsEmpty())
    return size_in_pixels_;
  return gfx::ScaleToFlooredSize(size(), device_scale_factor_);
}

std::string Display::ToString() const {
  return cr::StringPrintf(
      "Display[%lld] bounds=[%s], workarea=[%s], scale=%g, rotation=%s, "
      "panel_rotation=%s %s.",
      static_cast<long long int>(id_), bounds_.ToString().c_str(),
      work_area_.ToString().c_str(), device_scale_factor_,
      ToRotationString(rotation_), ToRotationString(panel_rotation()),
      IsInternal() ? "internal" : "external");
}

bool Display::IsInternal() const {
  return is_valid() && (id_ == internal_display_id_);
}

// static
int64_t Display::InternalDisplayId() {
  CR_DCHECK(kInvalidDisplayId != internal_display_id_);
  return internal_display_id_;
}

// static
void Display::SetInternalDisplayId(int64_t internal_display_id) {
  internal_display_id_ = internal_display_id;
}

// static
bool Display::IsInternalDisplayId(int64_t display_id) {
  CR_DCHECK(kInvalidDisplayId != display_id);
  return HasInternalDisplay() && internal_display_id_ == display_id;
}

// static
bool Display::HasInternalDisplay() {
  return internal_display_id_ != kInvalidDisplayId;
}

bool Display::operator==(const Display& rhs) const {
  return id_ == rhs.id_ && bounds_ == rhs.bounds_ &&
         size_in_pixels_ == rhs.size_in_pixels_ &&
         work_area_ == rhs.work_area_ &&
         device_scale_factor_ == rhs.device_scale_factor_ &&
         rotation_ == rhs.rotation_ && touch_support_ == rhs.touch_support_ &&
         accelerometer_support_ == rhs.accelerometer_support_ &&
         maximum_cursor_size_ == rhs.maximum_cursor_size_ &&
         ///color_spaces_ == rhs.color_spaces_ &&
         color_depth_ == rhs.color_depth_ &&
         depth_per_component_ == rhs.depth_per_component_ &&
         is_monochrome_ == rhs.is_monochrome_ &&
         display_frequency_ == rhs.display_frequency_;
}

}  // namespace display
}  // namespace crui
