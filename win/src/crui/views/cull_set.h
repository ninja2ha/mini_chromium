// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CULL_SET_H_
#define UI_VIEWS_CULL_SET_H_

#include <memory>
#include <unordered_set>

#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class View;

// A CullSet defines a set of View pointers which have been possibly culled
// from painting or other bounds-checking operations. It wraps a set of
// pointers to views, or NULL if no such set is available.
class CRUI_EXPORT CullSet {
 public:
  CullSet(const CullSet&) = delete;
  CullSet& operator=(const CullSet&) = delete;

  // Default constructor builds a CullSet that will always return true for
  // ShouldPaint().
  CullSet();
  ~CullSet();

  // Wraps a set of pointers to Views, as might be provided by
  // gfx::RTree::Query(), that intersect the damage rect and therefore need
  // to be painted. CullSet takes ownership of the provided pointer.
  CullSet(std::unique_ptr<std::unordered_set<intptr_t> > cull_set);

  // Returns true if |view| needs to be painted.
  bool ShouldPaint(const View* view) const;

 private:
  friend class BoundsTreeTestView;

  // The set of Views that collided with the query rectangle provided to the
  // RTree data structure, or NULL if one is not available.
  std::unique_ptr<std::unordered_set<intptr_t> > cull_set_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_CULL_SET_H_
