// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_WINDOW_UTIL_H_
#define UI_WM_CORE_WINDOW_UTIL_H_

#include <memory>
#include <utility>
#include <vector>

#include "crbase/functional/callback_forward.h"
#include "crbase/compiler_specific.h"
#include "crui/base/ui_base_types.h"
#include "crui/base/ui_export.h"

namespace crui {

///class Layer;
///class LayerOwner;
//class LayerTreeOwner;

namespace aura {
class Window;
}  // namespace aura

namespace wm {

CRUI_EXPORT void ActivateWindow(aura::Window* window);
CRUI_EXPORT void DeactivateWindow(aura::Window* window);
CRUI_EXPORT bool IsActiveWindow(const aura::Window* window);
CRUI_EXPORT bool CanActivateWindow(const aura::Window* window);
CRUI_EXPORT void SetWindowFullscreen(aura::Window* window, bool fullscreen);

// Returns true if |window|'s show state is |state|.
CRUI_EXPORT bool WindowStateIs(const aura::Window* window,
                               crui::WindowShowState state);

// Sets the window state to |state|.
CRUI_EXPORT void SetWindowState(aura::Window* window,
                                crui::WindowShowState state);

// Changes a window's state to its pre-minimized state.
CRUI_EXPORT void Unminimize(aura::Window* window);

// Retrieves the activatable window for |window|. If |window| is activatable,
// this will just return it, otherwise it will climb the parent/transient parent
// chain looking for a window that is activatable, per the ActivationClient.
// If you're looking for a function to get the activatable "top level" window,
// this is probably the function you're looking for.
CRUI_EXPORT aura::Window* GetActivatableWindow(aura::Window* window);

// Retrieves the toplevel window for |window|. The ActivationClient makes this
// determination.
CRUI_EXPORT aura::Window* GetToplevelWindow(aura::Window* window);
CRUI_EXPORT const aura::Window* GetToplevelWindow(
    const aura::Window* window);

// Returns the existing Layer for |root| (and all its descendants) and creates
// a new layer for |root| and all its descendants. This is intended for
// animations that want to animate between the existing visuals and a new state.
//
// As a result of this |root| has freshly created layers, meaning the layers
// have not yet been painted to.
///CRUI_EXPORT std::unique_ptr<crui::LayerTreeOwner> RecreateLayers(
///    crui::LayerOwner* root);

///using MapLayerFunc =
///    cr::RepeatingCallback<std::unique_ptr<crui::Layer>(crui::LayerOwner*)>;

// Maps |map_func| over each layer of the layer tree and returns a copy of the
// layer tree. The recursion stops at the level when |map_func| returns nullptr
// on the owner's layer. MapLayers might return nullptr when |map_func| returns
// nullptr on the root layer's owner.
///CRUI_EXPORT std::unique_ptr<crui::LayerTreeOwner> RecreateLayersWithClosure(
///    crui::LayerOwner* root,
///    const MapLayerFunc& map_func);

// Returns a layer tree that mirrors |root|. Used for live window previews. If
// |sync_bounds| is true, the bounds of all mirror layers except the root are
// synchronized. See |sync_bounds_with_source_| in ui::Layer.
///CRUI_EXPORT std::unique_ptr<crui::LayerTreeOwner> MirrorLayers(
///    crui::LayerOwner* root,
///    bool sync_bounds);

// Convenience functions that get the TransientWindowManager for the window and
// redirect appropriately. These are preferable to calling functions on
// TransientWindowManager as they handle the appropriate null checks.
CRUI_EXPORT aura::Window* GetTransientParent(aura::Window* window);
CRUI_EXPORT const aura::Window* GetTransientParent(
    const aura::Window* window);
CRUI_EXPORT const std::vector<aura::Window*>& GetTransientChildren(
    const aura::Window* window);
CRUI_EXPORT void AddTransientChild(aura::Window* parent,
                                   aura::Window* child);
CRUI_EXPORT void RemoveTransientChild(aura::Window* parent,
                                      aura::Window* child);
CRUI_EXPORT aura::Window* GetTransientRoot(aura::Window* window);

// Returns true if |window| has |ancestor| as a transient ancestor. A transient
// ancestor is found by following the transient parent chain of the window.
CRUI_EXPORT bool HasTransientAncestor(const aura::Window* window,
                                      const aura::Window* ancestor);

// Snap the window's layer to physical pixel boundary.
CRUI_EXPORT void SnapWindowToPixelBoundary(aura::Window* window);
}  // namespace wm

}  // namespace crui

#endif  // UI_WM_CORE_WINDOW_UTIL_H_
