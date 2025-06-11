// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CLIPBOARD_CLIPBOARD_MONITOR_H_
#define UI_BASE_CLIPBOARD_CLIPBOARD_MONITOR_H_

#include "crbase/memory/no_destructor.h"
#include "crbase/observer_list.h"
#include "crbase/threading/thread_checker.h"
#include "crui/base/ui_export.h"

namespace crui {

class ClipboardObserver;

// A singleton instance to monitor and notify ClipboardObservers for clipboard
// changes.
class CRUI_EXPORT ClipboardMonitor {
 public:
  static ClipboardMonitor* GetInstance();

  // Adds an observer.
  void AddObserver(ClipboardObserver* observer);

  // Removes an observer.
  void RemoveObserver(ClipboardObserver* observer);

  // Notifies all observers for clipboard data change.
  virtual void NotifyClipboardDataChanged();

  ClipboardMonitor (const ClipboardMonitor &) = delete;
  ClipboardMonitor & operator=(const ClipboardMonitor &) = delete;

 private:
  friend class cr::NoDestructor<ClipboardMonitor>;

  ClipboardMonitor();
  virtual ~ClipboardMonitor();

  ///cr::ObserverList<ClipboardObserver>::Unchecked observers_;
  cr::ObserverList<ClipboardObserver> observers_;

  CR_THREAD_CHECKER(thread_checker_)
};

}  // namespace crui

#endif  // UI_BASE_CLIPBOARD_CLIPBOARD_MONITOR_H_
