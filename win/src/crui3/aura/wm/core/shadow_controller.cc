// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//core/shadow_controller.h"

#include <utility>

///#include "crbase/command_line.h"
#include "crbase/containers/flat_set.h"
#include "crbase/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/scoped_observer.h"
#include "crbase/helper/stl_util.h"
#include "crui/aura/client/aura_constants.h"
#include "crui/aura/env.h"
#include "crui/aura/env_observer.h"
#include "crui/aura/window.h"
#include "crui/aura/window_observer.h"
#include "crui/base/class_property.h"
#include "crui/base/ui_base_types.h"
///#include "crui/compositor/layer.h"
///#include "crui/compositor_extra/shadow.h"
#include "crui/aura/wm//core/shadow_controller_delegate.h"
#include "crui/aura/wm//core/shadow_types.h"
#include "crui/aura/wm//core/window_util.h"
#include "crui/aura/wm//public/activation_client.h"

using std::make_pair;

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::Shadow*)
///DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(crui::Shadow, kShadowLayerKey, nullptr)

namespace crui {
namespace wm {

namespace {

int GetShadowElevationForActiveState(aura::Window* window) {
  int elevation = window->GetProperty(kShadowElevationKey);
  if (elevation != kShadowElevationDefault)
    return elevation;

  if (IsActiveWindow(window))
    return kShadowElevationActiveWindow;

  return GetDefaultShadowElevationForWindow(window);
}

// Returns the shadow style to be applied to |losing_active| when it is losing
// active to |gaining_active|. |gaining_active| may be of a type that hides when
// inactive, and as such we do not want to render |losing_active| as inactive.
int GetShadowElevationForWindowLosingActive(aura::Window* losing_active,
                                            aura::Window* gaining_active) {
  if (gaining_active && GetHideOnDeactivate(gaining_active)) {
    if (cr::Contains(GetTransientChildren(losing_active), gaining_active))
      return kShadowElevationActiveWindow;
  }
  return kShadowElevationInactiveWindow;
}

}  // namespace

// ShadowController::Impl ------------------------------------------------------

// Real implementation of the ShadowController. ShadowController observes
// ActivationChangeObserver, which are per ActivationClient, where as there is
// only a single Impl (as it observes all window creation by way of an
// EnvObserver).
class ShadowController::Impl :
      public aura::EnvObserver,
      public aura::WindowObserver,
      public cr::RefCounted<Impl> {
 public:
  // Returns the singleton instance for the specified Env.
  static Impl* GetInstance(aura::Env* env);

  void set_delegate(std::unique_ptr<ShadowControllerDelegate> delegate) {
    delegate_ = std::move(delegate);
  }
  bool IsShadowVisibleForWindow(aura::Window* window);
  void UpdateShadowForWindow(aura::Window* window);

  // aura::EnvObserver override:
  void OnWindowInitialized(aura::Window* window) override;

  // aura::WindowObserver overrides:
  void OnWindowParentChanged(aura::Window* window,
                             aura::Window* parent) override;
  void OnWindowPropertyChanged(aura::Window* window,
                               const void* key,
                               intptr_t old) override;
  void OnWindowVisibilityChanging(aura::Window* window, bool visible) override;
  void OnWindowBoundsChanged(aura::Window* window,
                             const gfx::Rect& old_bounds,
                             const gfx::Rect& new_bounds,
                             crui::PropertyChangeReason reason) override;
  void OnWindowDestroyed(aura::Window* window) override;

 private:
  friend class cr::RefCounted<Impl>;
  friend class ShadowController;

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  explicit Impl(aura::Env* env);
  ~Impl() override;

  static cr::flat_set<Impl*>* GetInstances();

  // Forwarded from ShadowController.
  void OnWindowActivated(ActivationReason reason,
                         aura::Window* gained_active,
                         aura::Window* lost_active);

  // Checks if |window| is visible and contains a property requesting a shadow.
  bool ShouldShowShadowForWindow(aura::Window* window) const;

  // Updates the shadow for windows when activation changes.
  void HandleWindowActivationChange(aura::Window* gaining_active,
                                    aura::Window* losing_active);

  // Shows or hides |window|'s shadow as needed (creating the shadow if
  // necessary).
  void HandlePossibleShadowVisibilityChange(aura::Window* window);

  // Creates a new shadow for |window| and stores it with the |kShadowLayerKey|
  // key.
  // The shadow's bounds are initialized and it is added to the window's layer.
  void CreateShadowForWindow(aura::Window* window);

  aura::Env* const env_;
  cr::ScopedObserver<aura::Window, aura::WindowObserver> observer_manager_;

  std::unique_ptr<ShadowControllerDelegate> delegate_;
};

// static
ShadowController::Impl* ShadowController::Impl::GetInstance(aura::Env* env) {
  for (Impl* impl : *GetInstances()) {
    if (impl->env_ == env)
      return impl;
  }

  return new Impl(env);
}

bool ShadowController::Impl::IsShadowVisibleForWindow(aura::Window* window) {
  if (!observer_manager_.IsObserving(window))
    return false;
  ///crui::Shadow* shadow = GetShadowForWindow(window);
  ///return shadow && shadow->layer()->visible();
  CR_NOTIMPLEMENTED();
  return false;
}

void ShadowController::Impl::UpdateShadowForWindow(aura::Window* window) {
  CR_DCHECK(observer_manager_.IsObserving(window));
  HandlePossibleShadowVisibilityChange(window);
}

void ShadowController::Impl::OnWindowInitialized(aura::Window* window) {
  // During initialization, the window can't reliably tell whether it will be a
  // root window. That must be checked in the first visibility change
  CR_DCHECK(!window->parent());
  CR_DCHECK(!window->TargetVisibility());
  observer_manager_.Add(window);
}

void ShadowController::Impl::OnWindowParentChanged(aura::Window* window,
                                                   aura::Window* parent) {
  if (parent && window->IsVisible())
    HandlePossibleShadowVisibilityChange(window);
}

void ShadowController::Impl::OnWindowPropertyChanged(aura::Window* window,
                                                     const void* key,
                                                     intptr_t old) {
  bool shadow_will_change = key == kShadowElevationKey &&
                            window->GetProperty(kShadowElevationKey) != old;

  if (key == aura::client::kShowStateKey) {
    shadow_will_change = window->GetProperty(aura::client::kShowStateKey) !=
                         static_cast<crui::WindowShowState>(old);
  }

  // Check the target visibility. IsVisible() may return false if a parent layer
  // is hidden, but |this| only observes calls to Show()/Hide() on |window|.
  if (shadow_will_change && window->TargetVisibility())
    HandlePossibleShadowVisibilityChange(window);
}

void ShadowController::Impl::OnWindowVisibilityChanging(aura::Window* window,
                                                        bool visible) {
  // At the time of the first visibility change, |window| will give a correct
  // answer for whether or not it is a root window. If it is, don't bother
  // observing: a shadow should never be added. Root windows can only have
  // shadows in the WindowServer (where a corresponding aura::Window may no
  // longer be a root window). Without this check, a second shadow is added,
  // which clips to the root window bounds; filling any rounded corners the
  // window may have.
  if (window->IsRootWindow()) {
    observer_manager_.Remove(window);
    return;
  }

  HandlePossibleShadowVisibilityChange(window);
}

void ShadowController::Impl::OnWindowBoundsChanged(
    aura::Window* window,
    const gfx::Rect& old_bounds,
    const gfx::Rect& new_bounds,
    crui::PropertyChangeReason reason) {
  ///crui::Shadow* shadow = GetShadowForWindow(window);
  ///if (shadow)
  ///  shadow->SetContentBounds(gfx::Rect(new_bounds.size()));
}

void ShadowController::Impl::OnWindowDestroyed(aura::Window* window) {
  window->ClearProperty(kShadowLayerKey);
  observer_manager_.Remove(window);
}

void ShadowController::Impl::OnWindowActivated(ActivationReason reason,
                                               aura::Window* gained_active,
                                               aura::Window* lost_active) {
  if (gained_active) {
    crui::Shadow* shadow = GetShadowForWindow(gained_active);
    if (shadow)
      shadow->SetElevation(GetShadowElevationForActiveState(gained_active));
  }
  if (lost_active) {
    crui::Shadow* shadow = GetShadowForWindow(lost_active);
    if (shadow && GetShadowElevationConvertDefault(lost_active) ==
                      kShadowElevationInactiveWindow) {
      shadow->SetElevation(
          GetShadowElevationForWindowLosingActive(lost_active, gained_active));
    }
  }
}

bool ShadowController::Impl::ShouldShowShadowForWindow(
    aura::Window* window) const {
  if (delegate_) {
    const bool should_show = delegate_->ShouldShowShadowForWindow(window);
    if (should_show)
      CR_DCHECK(GetShadowElevationConvertDefault(window) > 0);
    return should_show;
  }

  crui::WindowShowState show_state =
      window->GetProperty(aura::client::kShowStateKey);
  if (show_state == crui::SHOW_STATE_FULLSCREEN ||
      show_state == crui::SHOW_STATE_MAXIMIZED) {
    return false;
  }

  return GetShadowElevationConvertDefault(window) > 0;
}

void ShadowController::Impl::HandlePossibleShadowVisibilityChange(
    aura::Window* window) {
  const bool should_show = ShouldShowShadowForWindow(window);
  crui::Shadow* shadow = GetShadowForWindow(window);
  if (shadow) {
    shadow->SetElevation(GetShadowElevationForActiveState(window));
    shadow->layer()->SetVisible(should_show);
  } else if (should_show && !shadow) {
    CreateShadowForWindow(window);
  }
}

void ShadowController::Impl::CreateShadowForWindow(aura::Window* window) {
  CR_DCHECK(!window->IsRootWindow());
  crui::Shadow* shadow = new crui::Shadow();
  window->SetProperty(kShadowLayerKey, shadow);

  int corner_radius = window->GetProperty(aura::client::kWindowCornerRadiusKey);
  if (corner_radius >= 0)
    shadow->SetRoundedCornerRadius(corner_radius);

  shadow->Init(GetShadowElevationForActiveState(window));
  shadow->SetContentBounds(gfx::Rect(window->bounds().size()));
  shadow->layer()->SetVisible(ShouldShowShadowForWindow(window));
  window->layer()->Add(shadow->layer());
  window->layer()->StackAtBottom(shadow->layer());
}

ShadowController::Impl::Impl(aura::Env* env)
    : env_(env), observer_manager_(this) {
  GetInstances()->insert(this);
  env_->AddObserver(this);
}

ShadowController::Impl::~Impl() {
  env_->RemoveObserver(this);
  GetInstances()->erase(this);
}

// static
cr::flat_set<ShadowController::Impl*>*
ShadowController::Impl::GetInstances() {
  static cr::NoDestructor<cr::flat_set<Impl*>> impls;
  return impls.get();
}

// ShadowController ------------------------------------------------------------

crui::Shadow* ShadowController::GetShadowForWindow(aura::Window* window) {
  return window->GetProperty(kShadowLayerKey);
}

ShadowController::ShadowController(
    ActivationClient* activation_client,
    std::unique_ptr<ShadowControllerDelegate> delegate,
    aura::Env* env)
    : activation_client_(activation_client),
      impl_(Impl::GetInstance(env ? env : aura::Env::GetInstance())) {
  // Watch for window activation changes.
  activation_client_->AddObserver(this);
  if (delegate)
    impl_->set_delegate(std::move(delegate));
}

ShadowController::~ShadowController() {
  activation_client_->RemoveObserver(this);
}

bool ShadowController::IsShadowVisibleForWindow(aura::Window* window) {
  return impl_->IsShadowVisibleForWindow(window);
}

void ShadowController::UpdateShadowForWindow(aura::Window* window) {
  impl_->UpdateShadowForWindow(window);
}

void ShadowController::OnWindowActivated(ActivationReason reason,
                                         aura::Window* gained_active,
                                         aura::Window* lost_active) {
  impl_->OnWindowActivated(reason, gained_active, lost_active);
}

}  // namespace wm
}  // namespace crui
