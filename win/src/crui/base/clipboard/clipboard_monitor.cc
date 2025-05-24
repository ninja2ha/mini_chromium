// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/clipboard/clipboard_monitor.h"

#include "crui/base/clipboard/clipboard.h"
#include "crui/base/clipboard/clipboard_observer.h"

namespace crui {

ClipboardMonitor::ClipboardMonitor() = default;

ClipboardMonitor::~ClipboardMonitor() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
}

// static
ClipboardMonitor* ClipboardMonitor::GetInstance() {
  static cr::NoDestructor<ClipboardMonitor> monitor;
  return monitor.get();
}

void ClipboardMonitor::NotifyClipboardDataChanged() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  for (ClipboardObserver& observer : observers_)
    observer.OnClipboardDataChanged();
}

void ClipboardMonitor::AddObserver(ClipboardObserver* observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  observers_.AddObserver(observer);
}

void ClipboardMonitor::RemoveObserver(ClipboardObserver* observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  observers_.RemoveObserver(observer);
}

}  // namespace crui
