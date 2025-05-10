// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/cursor/cursors_aura.h"

#include <stddef.h>

#include "crbase/helper/stl_util.h"
///#include "crthird_party/skia/include/core/SkBitmap.h"
///#include "crui/base/resource/resource_bundle.h"
#include "crui/gfx/geometry/point.h"
///#include "crui/gfx/image/image_skia.h"
///#include "crui/resources/grit/ui_resources.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crui/base/cursor/cursor_loader_win.h"
///#include "crui/gfx/icon_util.h"
#endif

namespace crui {
namespace {

struct HotPoint {
  int x;
  int y;
};

struct CursorData {
  CursorType id;
  int resource_id;
  HotPoint hot_1x;
  HotPoint hot_2x;
};

struct CursorSizeData {
  const CursorSize id;
  const CursorData* cursors;
  const int length;
  const CursorData* animated_cursors;
  const int animated_length;
};

///const CursorData kNormalCursors[] = {
///    {CursorType::kNull, IDR_AURA_CURSOR_PTR, {4, 4}, {7, 7}},
///    {CursorType::kPointer, IDR_AURA_CURSOR_PTR, {4, 4}, {7, 7}},
///    {CursorType::kNoDrop, IDR_AURA_CURSOR_NO_DROP, {9, 9}, {18, 18}},
///    {CursorType::kNotAllowed, IDR_AURA_CURSOR_NO_DROP, {9, 9}, {18, 18}},
///    {CursorType::kCopy, IDR_AURA_CURSOR_COPY, {9, 9}, {18, 18}},
///    {CursorType::kHand, IDR_AURA_CURSOR_HAND, {9, 4}, {19, 8}},
///    {CursorType::kMove, IDR_AURA_CURSOR_MOVE, {11, 11}, {23, 23}},
///    {CursorType::kNorthEastResize,
///     IDR_AURA_CURSOR_NORTH_EAST_RESIZE,
///     {12, 11},
///     {25, 23}},
///    {CursorType::kSouthWestResize,
///     IDR_AURA_CURSOR_SOUTH_WEST_RESIZE,
///     {12, 11},
///     {25, 23}},
///    {CursorType::kSouthEastResize,
///     IDR_AURA_CURSOR_SOUTH_EAST_RESIZE,
///     {11, 11},
///     {24, 23}},
///    {CursorType::kNorthWestResize,
///     IDR_AURA_CURSOR_NORTH_WEST_RESIZE,
///     {11, 11},
///     {24, 23}},
///    {CursorType::kNorthResize,
///     IDR_AURA_CURSOR_NORTH_RESIZE,
///     {11, 12},
///     {23, 23}},
///    {CursorType::kSouthResize,
///     IDR_AURA_CURSOR_SOUTH_RESIZE,
///     {11, 12},
///     {23, 23}},
///    {CursorType::kEastResize, IDR_AURA_CURSOR_EAST_RESIZE, {12, 11}, {25, 23}},
///    {CursorType::kWestResize, IDR_AURA_CURSOR_WEST_RESIZE, {12, 11}, {25, 23}},
///    {CursorType::kIBeam, IDR_AURA_CURSOR_IBEAM, {12, 12}, {24, 25}},
///    {CursorType::kAlias, IDR_AURA_CURSOR_ALIAS, {8, 6}, {15, 11}},
///    {CursorType::kCell, IDR_AURA_CURSOR_CELL, {11, 11}, {24, 23}},
///    {CursorType::kContextMenu, IDR_AURA_CURSOR_CONTEXT_MENU, {4, 4}, {8, 9}},
///    {CursorType::kCross, IDR_AURA_CURSOR_CROSSHAIR, {12, 12}, {24, 24}},
///    {CursorType::kHelp, IDR_AURA_CURSOR_HELP, {4, 4}, {8, 9}},
///    {CursorType::kVerticalText,
///     IDR_AURA_CURSOR_XTERM_HORIZ,
///     {12, 11},
///     {26, 23}},
///    {CursorType::kZoomIn, IDR_AURA_CURSOR_ZOOM_IN, {10, 10}, {20, 20}},
///    {CursorType::kZoomOut, IDR_AURA_CURSOR_ZOOM_OUT, {10, 10}, {20, 20}},
///    {CursorType::kRowResize, IDR_AURA_CURSOR_ROW_RESIZE, {11, 12}, {23, 23}},
///    {CursorType::kColumnResize, IDR_AURA_CURSOR_COL_RESIZE, {12, 11}, {25, 23}},
///    {CursorType::kEastWestResize,
///     IDR_AURA_CURSOR_EAST_WEST_RESIZE,
///     {12, 11},
///     {25, 23}},
///    {CursorType::kNorthSouthResize,
///     IDR_AURA_CURSOR_NORTH_SOUTH_RESIZE,
///     {11, 12},
///     {23, 23}},
///    {CursorType::kNorthEastSouthWestResize,
///     IDR_AURA_CURSOR_NORTH_EAST_SOUTH_WEST_RESIZE,
///     {12, 11},
///     {25, 23}},
///    {CursorType::kNorthWestSouthEastResize,
///     IDR_AURA_CURSOR_NORTH_WEST_SOUTH_EAST_RESIZE,
///     {11, 11},
///     {24, 23}},
///    {CursorType::kGrab, IDR_AURA_CURSOR_GRAB, {8, 5}, {16, 10}},
///    {CursorType::kGrabbing, IDR_AURA_CURSOR_GRABBING, {9, 9}, {18, 18}},
///};
///
///const CursorData kAnimatedCursors[] = {
///    {CursorType::kWait, IDR_AURA_CURSOR_THROBBER, {7, 7}, {14, 14}},
///    {CursorType::kProgress, IDR_AURA_CURSOR_THROBBER, {7, 7}, {14, 14}},
///};
///
///const CursorSizeData kCursorSizes[] = {
///    {CursorSize::kNormal, kNormalCursors, cr::size(kNormalCursors),
///     kAnimatedCursors, cr::size(kAnimatedCursors)},
///    {CursorSize::kLarge, kLargeCursors, cr::size(kLargeCursors),
///     // TODO(yoshiki): Replace animated cursors with big assets.
///     // crbug.com/247254
///     kAnimatedCursors, cr::size(kAnimatedCursors)},
///};

const CursorSizeData* GetCursorSizeByType(CursorSize cursor_size) {
  ///for (size_t i = 0; i < cr::size(kCursorSizes); ++i) {
  ///  if (kCursorSizes[i].id == cursor_size)
  ///    return &kCursorSizes[i];
  ///}
  ///
  return NULL;
}

bool SearchTable(const CursorData* table,
                 size_t table_length,
                 CursorType id,
                 float scale_factor,
                 int* resource_id,
                 gfx::Point* point) {
  CR_DCHECK(scale_factor, 0);

  ///bool resource_2x_available =
  ///    ResourceBundle::GetSharedInstance().GetMaxScaleFactor() ==
  ///    SCALE_FACTOR_200P;
  bool resource_2x_available = false;
  for (size_t i = 0; i < table_length; ++i) {
    if (table[i].id == id) {
      *resource_id = table[i].resource_id;
      *point = scale_factor == 1.0f || !resource_2x_available
                   ? gfx::Point(table[i].hot_1x.x, table[i].hot_1x.y)
                   : gfx::Point(table[i].hot_2x.x, table[i].hot_2x.y);
      return true;
    }
  }

  return false;
}

}  // namespace

const char* CursorCssNameFromId(CursorType id) {
  switch (id) {
    case CursorType::kMiddlePanning:
      return "all-scroll";
    case CursorType::kMiddlePanningVertical:
      return "v-scroll";
    case CursorType::kMiddlePanningHorizontal:
      return "h-scroll";
    case CursorType::kEastPanning:
      return "e-resize";
    case CursorType::kNorthPanning:
      return "n-resize";
    case CursorType::kNorthEastPanning:
      return "ne-resize";
    case CursorType::kNorthWestPanning:
      return "nw-resize";
    case CursorType::kSouthPanning:
      return "s-resize";
    case CursorType::kSouthEastPanning:
      return "se-resize";
    case CursorType::kSouthWestPanning:
      return "sw-resize";
    case CursorType::kWestPanning:
      return "w-resize";
    case CursorType::kNone:
      return "none";
    case CursorType::kGrab:
      return "grab";
    case CursorType::kGrabbing:
      return "grabbing";
    case CursorType::kNull:
      return "left_ptr";
    case CursorType::kPointer:
      return "left_ptr";
    case CursorType::kMove:
      // Returning "move" is the correct thing here, but Blink doesn't
      // make a distinction between move and all-scroll.  Other
      // platforms use a cursor more consistent with all-scroll, so
      // use that.
      return "all-scroll";
    case CursorType::kCross:
      return "crosshair";
    case CursorType::kHand:
      return "pointer";
    case CursorType::kIBeam:
      return "text";
    case CursorType::kProgress:
      return "progress";
    case CursorType::kWait:
      return "wait";
    case CursorType::kHelp:
      return "help";
    case CursorType::kEastResize:
      return "e-resize";
    case CursorType::kNorthResize:
      return "n-resize";
    case CursorType::kNorthEastResize:
      return "ne-resize";
    case CursorType::kNorthWestResize:
      return "nw-resize";
    case CursorType::kSouthResize:
      return "s-resize";
    case CursorType::kSouthEastResize:
      return "se-resize";
    case CursorType::kSouthWestResize:
      return "sw-resize";
    case CursorType::kWestResize:
      return "w-resize";
    case CursorType::kNorthSouthResize:
      return "ns-resize";
    case CursorType::kEastWestResize:
      return "ew-resize";
    case CursorType::kColumnResize:
      return "col-resize";
    case CursorType::kRowResize:
      return "row-resize";
    case CursorType::kNorthEastSouthWestResize:
      return "nesw-resize";
    case CursorType::kNorthWestSouthEastResize:
      return "nwse-resize";
    case CursorType::kVerticalText:
      return "vertical-text";
    case CursorType::kZoomIn:
      return "zoom-in";
    case CursorType::kZoomOut:
      return "zoom-out";
    case CursorType::kCell:
      return "cell";
    case CursorType::kContextMenu:
      return "context-menu";
    case CursorType::kAlias:
      return "alias";
    case CursorType::kNoDrop:
      return "no-drop";
    case CursorType::kCopy:
      return "copy";
    case CursorType::kNotAllowed:
      return "not-allowed";
    case CursorType::kDndNone:
      return "dnd-none";
    case CursorType::kDndMove:
      return "dnd-move";
    case CursorType::kDndCopy:
      return "dnd-copy";
    case CursorType::kDndLink:
      return "dnd-link";
    case CursorType::kCustom:
      CR_NOTREACHED();
      return "left_ptr";
  }
  CR_NOTREACHED() << "Case not handled for " << static_cast<int>(id);
  return "left_ptr";
}

bool GetCursorDataFor(CursorSize cursor_size,
                      CursorType id,
                      float scale_factor,
                      int* resource_id,
                      gfx::Point* point) {
  const CursorSizeData* cursor_set = GetCursorSizeByType(cursor_size);
  if (cursor_set && SearchTable(cursor_set->cursors, cursor_set->length, id,
                                scale_factor, resource_id, point)) {
    return true;
  }

  // Falls back to the default cursor set.
  cursor_set = GetCursorSizeByType(crui::CursorSize::kNormal);
  CR_DCHECK(cursor_set);
  return SearchTable(cursor_set->cursors, cursor_set->length, id, scale_factor,
                     resource_id, point);
}

bool GetAnimatedCursorDataFor(CursorSize cursor_size,
                              CursorType id,
                              float scale_factor,
                              int* resource_id,
                              gfx::Point* point) {
  const CursorSizeData* cursor_set = GetCursorSizeByType(cursor_size);
  if (cursor_set &&
      SearchTable(cursor_set->animated_cursors, cursor_set->animated_length, id,
                  scale_factor, resource_id, point)) {
    return true;
  }

  // Falls back to the default cursor set.
  cursor_set = GetCursorSizeByType(crui::CursorSize::kNormal);
  CR_DCHECK(cursor_set);
  return SearchTable(cursor_set->animated_cursors, cursor_set->animated_length,
                     id, scale_factor, resource_id, point);
}

///SkBitmap Cursor::GetDefaultBitmap() const {
///#if defined(MINI_CHROMIUM_OS_WIN)
///  Cursor cursor_copy = *this;
///  crui::CursorLoaderWin cursor_loader;
///  cursor_loader.SetPlatformCursor(&cursor_copy);
///  return IconUtil::CreateSkBitmapFromHICON(cursor_copy.platform());
///#else
///  int resource_id;
///  gfx::Point hotspot;
///  if (!GetCursorDataFor(ui::CursorSize::kNormal, native_type(),
///                        device_scale_factor(), &resource_id, &hotspot)) {
///    return SkBitmap();
///  }
///  return *ResourceBundle::GetSharedInstance()
///              .GetImageSkiaNamed(resource_id)
///              ->bitmap();
///#endif
///}

///gfx::Point Cursor::GetDefaultHotspot() const {
///#if defined(MINI_CHROMIUM_OS_WIN)
///  Cursor cursor_copy = *this;
///  ui::CursorLoaderWin cursor_loader;
///  cursor_loader.SetPlatformCursor(&cursor_copy);
///  return IconUtil::GetHotSpotFromHICON(cursor_copy.platform());
///#else
///  int resource_id;
///  gfx::Point hotspot;
///  if (!GetCursorDataFor(ui::CursorSize::kNormal, native_type(),
///                        device_scale_factor(), &resource_id, &hotspot)) {
///    return gfx::Point();
///  }
///  return hotspot;
///#endif
///}

}  // namespace crui
