// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/event_target.h"

#include <algorithm>

#include "crbase/logging.h"
#include "crui/events/event.h"
#include "crui/gfx/geometry/point_conversions.h"

namespace crui {

EventTarget::EventTarget() = default;

EventTarget::~EventTarget() = default;

void EventTarget::ConvertEventToTarget(const EventTarget* target,
                                       LocatedEvent* event) const {}

gfx::PointF EventTarget::GetScreenLocationF(
    const crui::LocatedEvent& event) const {
  CR_NOTREACHED();
  return event.root_location_f();
}

gfx::Point EventTarget::GetScreenLocation(
    const crui::LocatedEvent& event) const {
  return gfx::ToFlooredPoint(GetScreenLocationF(event));
}

void EventTarget::AddPreTargetHandler(EventHandler* handler,
                                      Priority priority) {
  CR_CHECK(handler);
  PrioritizedHandler prioritized;
  prioritized.handler = handler;
  prioritized.priority = priority;
  if (priority == Priority::kDefault)
    pre_target_list_.push_back(prioritized);
  else
    pre_target_list_.insert(pre_target_list_.begin(), prioritized);
  handler->targets_installed_on_.push_back(this);
}

void EventTarget::RemovePreTargetHandler(EventHandler* handler) {
  CR_CHECK(handler);
  // Only erase a single one, which matches the removal code right after this.
  auto installed_on_iter =
      std::find(handler->targets_installed_on_.begin(),
                handler->targets_installed_on_.end(), this);
  if (installed_on_iter != handler->targets_installed_on_.end())
    handler->targets_installed_on_.erase(installed_on_iter);

  EventHandlerPriorityList::iterator it, end;
  for (it = pre_target_list_.begin(), end = pre_target_list_.end(); it != end;
       ++it) {
    if (it->handler == handler) {
      pre_target_list_.erase(it);
      return;
    }
  }
}

void EventTarget::AddPostTargetHandler(EventHandler* handler) {
  CR_DCHECK(handler);
  post_target_list_.push_back(handler);
}

void EventTarget::RemovePostTargetHandler(EventHandler* handler) {
  auto find =
      std::find(post_target_list_.begin(), post_target_list_.end(), handler);
  if (find != post_target_list_.end())
    post_target_list_.erase(find);
}

bool EventTarget::IsPreTargetListEmpty() const {
  return pre_target_list_.empty();
}

EventHandler* EventTarget::SetTargetHandler(EventHandler* target_handler) {
  EventHandler* original_target_handler = target_handler_;
  target_handler_ = target_handler;
  return original_target_handler;
}

void EventTarget::GetPreTargetHandlers(EventHandlerList* list) {
  EventTarget* target = this;
  EventHandlerPriorityList temp;
  while (target) {
    // Build a composite list of EventHandlers from targets.
    temp.insert(temp.begin(), target->pre_target_list_.begin(),
                target->pre_target_list_.end());
    target = target->GetParentTarget();
  }

  // Sort the list, keeping relative order, but making sure the
  // accessibility handlers always go first before system, which will
  // go before default, at all levels of EventTarget.
  std::stable_sort(temp.begin(), temp.end());

  // Add the sorted handlers to the result list, in order.
  for (size_t i = 0; i < temp.size(); ++i)
    list->insert(list->end(), temp[i].handler);
}

void EventTarget::GetPostTargetHandlers(EventHandlerList* list) {
  EventTarget* target = this;
  while (target) {
    list->insert(list->end(), target->post_target_list_.begin(),
                 target->post_target_list_.end());
    target = target->GetParentTarget();
  }
}

}  // namespace crui
