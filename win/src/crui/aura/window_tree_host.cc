// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/window_tree_host.h"

///#include "base/command_line.h"
///#include "base/feature_list.h"
///#include "base/metrics/histogram_macros.h"
#include "crbase/threading/thread_task_runner_handle.h"
///#include "base/trace_event/trace_event.h"
///#include "components/viz/common/features.h"
#include "crui/aura/client/capture_client.h"
#include "crui/aura/client/cursor_client.h"
#include "crui/aura/env.h"
#include "crui/aura/scoped_keyboard_hook.h"
#include "crui/aura/scoped_simple_keyboard_hook.h"
#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/aura/window_targeter.h"
#include "crui/aura/window_tree_host_observer.h"
///#include "crui/base/ime/init/input_method_factory.h"
///#include "crui/base/ime/input_method.h"
///#include "crui/base/layout.h"
///#include "crui/base/ui_base_features.h"
#include "crui/base/view_prop.h"
///#include "crui/compositor/compositor_switches.h"
///#include "crui/compositor/dip_util.h"
///#include "crui/compositor/layer.h"
#include "crui/display/display.h"
#include "crui/display/screen.h"
#include "crui/events/keycodes/dom/dom_code.h"
#include "crui/gfx/geometry/insets.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/point3_f.h"
#include "crui/gfx/geometry/point_conversions.h"
#include "crui/gfx/geometry/rect_conversions.h"
#include "crui/gfx/geometry/size_conversions.h"
#include "crui/gfx/geometry/transform.h"
///#include "crui/gfx/icc_profile.h"
///#include "crui/gfx/switches.h"
#include "crui/aura/platform_window/platform_window_init_properties.h"

#if defined(MINI_CHROMIUM_OS_WIN)
///#include "crui/aura/native_window_occlusion_tracker_win.h"
#endif  // MINI_CHROMIUM_OS_WIN

namespace crui {
namespace aura {

namespace {

const char kWindowTreeHostForAcceleratedWidget[] =
    "__AURA_WINDOW_TREE_HOST_ACCELERATED_WIDGET__";

#if CR_DCHECK_IS_ON()
class ScopedLocalSurfaceIdValidator {
 public:
  ScopedLocalSurfaceIdValidator(
      const ScopedLocalSurfaceIdValidator&) = delete;
  ScopedLocalSurfaceIdValidator& operator=(
      const ScopedLocalSurfaceIdValidator&) = delete;

  explicit ScopedLocalSurfaceIdValidator(Window* window)
      : window_(window)/*,
        local_surface_id_(
            window ? window->GetLocalSurfaceIdAllocation().local_surface_id()
                   : viz::LocalSurfaceId())*/ {}
  ~ScopedLocalSurfaceIdValidator() {
    if (window_) {
      ///CR_DCHECK(local_surface_id_ ==
      ///          window_->GetLocalSurfaceIdAllocation().local_surface_id());
    }
  }

 private:
  Window* const window_;
  ///const viz::LocalSurfaceId local_surface_id_;
};
#else
class ScopedLocalSurfaceIdValidator {
 public:
  explicit ScopedLocalSurfaceIdValidator(Window* window) {}
  ~ScopedLocalSurfaceIdValidator() {}
};
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
bool IsNativeWindowOcclusionEnabled() {
  ///return !cr::CommandLine::ForCurrentProcess()->HasSwitch(
  ///           switches::kHeadless) &&
  ///       cr::FeatureList::IsEnabled(features::kCalculateNativeWinOcclusion);
  return false;
} 
#endif  // MINI_CHROMIUM_OS_WIN

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHost, public:

WindowTreeHost::~WindowTreeHost() {
  if (display::Screen::GetScreen())
    display::Screen::GetScreen()->RemoveObserver(this);
  ///CR_DCHECK(!compositor_) << "compositor must be destroyed before root window";
  ///if (owned_input_method_) {
  ///  delete input_method_;
  ///  input_method_ = nullptr;
  ///}
}

// static
WindowTreeHost* WindowTreeHost::GetForAcceleratedWidget(
    gfx::AcceleratedWidget widget) {
  return reinterpret_cast<WindowTreeHost*>(
      crui::ViewProp::GetValue(widget, kWindowTreeHostForAcceleratedWidget));
}

void WindowTreeHost::InitHost() {
  display::Display display =
      display::Screen::GetScreen()->GetDisplayNearestWindow(window());
  device_scale_factor_ = display.device_scale_factor();

  UpdateRootWindowSizeInPixels();
  InitCompositor();
  Env::GetInstance()->NotifyHostInitialized(this);
}

void WindowTreeHost::AddObserver(WindowTreeHostObserver* observer) {
  observers_.AddObserver(observer);
}

void WindowTreeHost::RemoveObserver(WindowTreeHostObserver* observer) {
  observers_.RemoveObserver(observer);
}

bool WindowTreeHost::HasObserver(const WindowTreeHostObserver* observer) const {
  return observers_.HasObserver(observer);
}

crui::EventSink* WindowTreeHost::event_sink() {
  return dispatcher_.get();
}

cr::WeakPtr<WindowTreeHost> WindowTreeHost::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

gfx::Transform WindowTreeHost::GetRootTransform() const {
  gfx::Transform transform;
  transform.Scale(device_scale_factor_, device_scale_factor_);
  ///transform *= window()->layer()->transform();
  return transform;
}

void WindowTreeHost::SetRootTransform(const gfx::Transform& transform) {
  window()->SetTransform(transform);
  UpdateRootWindowSizeInPixels();
}

gfx::Transform WindowTreeHost::GetInverseRootTransform() const {
  gfx::Transform invert;
  gfx::Transform transform = GetRootTransform();
  if (!transform.GetInverse(&invert))
    return transform;
  return invert;
}

///void WindowTreeHost::SetDisplayTransformHint(gfx::OverlayTransform transform) {
///  if (compositor()->display_transform_hint() == transform)
///    return;
///
///  compositor()->SetDisplayTransformHint(transform);
///  UpdateCompositorScaleAndSize(GetBoundsInPixels().size());
///}

gfx::Transform WindowTreeHost::GetRootTransformForLocalEventCoordinates()
    const {
  return GetRootTransform();
}

gfx::Transform WindowTreeHost::GetInverseRootTransformForLocalEventCoordinates()
    const {
  gfx::Transform invert;
  gfx::Transform transform = GetRootTransformForLocalEventCoordinates();
  if (!transform.GetInverse(&invert))
    return transform;
  return invert;
}

void WindowTreeHost::UpdateRootWindowSizeInPixels() {
  // Validate that the LocalSurfaceId does not change.
  ///bool compositor_inited = !!compositor()->root_layer();
  ///bool compositor_inited = false;
  ///ScopedLocalSurfaceIdValidator lsi_validator(compositor_inited ? window()
  ///                                                              : nullptr);
  gfx::Rect transformed_bounds_in_pixels =
      GetTransformedRootWindowBoundsInPixels(GetBoundsInPixels().size());
  window()->SetBounds(transformed_bounds_in_pixels);
}

void WindowTreeHost::UpdateCompositorScaleAndSize(
    const gfx::Size& new_size_in_pixels) {
  ///gfx::Rect new_bounds(new_size_in_pixels);
  ///if (compositor_->display_transform_hint() ==
  ///        gfx::OVERLAY_TRANSFORM_ROTATE_90 ||
  ///    compositor_->display_transform_hint() ==
  ///        gfx::OVERLAY_TRANSFORM_ROTATE_270) {
  ///  new_bounds.Transpose();
  ///}
  ///
  // Allocate a new LocalSurfaceId for the new size or scale factor.
  ///window_->AllocateLocalSurfaceId();
  ///ScopedLocalSurfaceIdValidator lsi_validator(window());
 /// compositor_->SetScaleAndSize(device_scale_factor_, new_bounds.size(),
 ///                              window_->GetLocalSurfaceIdAllocation());
}

void WindowTreeHost::ConvertDIPToScreenInPixels(gfx::Point* point) const {
  ConvertDIPToPixels(point);
  gfx::Point location = GetLocationOnScreenInPixels();
  point->Offset(location.x(), location.y());
}

void WindowTreeHost::ConvertScreenInPixelsToDIP(gfx::Point* point) const {
  gfx::Point location = GetLocationOnScreenInPixels();
  point->Offset(-location.x(), -location.y());
  ConvertPixelsToDIP(point);
}

void WindowTreeHost::ConvertDIPToPixels(gfx::Point* point) const {
  ///auto point_3f = gfx::Point3F(gfx::PointF(*point));
  ///GetRootTransform().TransformPoint(&point_3f);
  ///*point = gfx::ToFlooredPoint(point_3f.AsPointF());
  gfx::PointF point_f{ *point };
  point_f = GetRootTransform().MapPoint(point_f);
  *point = gfx::ToFlooredPoint(point_f);
}

void WindowTreeHost::ConvertPixelsToDIP(gfx::Point* point) const {
  ///auto point_3f = gfx::Point3F(gfx::PointF(*point));
  ///GetInverseRootTransform().TransformPoint(&point_3f);
  ///*point = gfx::ToFlooredPoint(point_3f.AsPointF());
  gfx::PointF point_f{ *point };
  point_f = GetInverseRootTransform().MapPoint(point_f);
  *point = gfx::ToFlooredPoint(point_f);
}

void WindowTreeHost::SetCursor(gfx::NativeCursor cursor) {
  last_cursor_ = cursor;
  // A lot of code seems to depend on NULL cursors actually showing an arrow,
  // so just pass everything along to the host.
  SetCursorNative(cursor);
}

void WindowTreeHost::OnCursorVisibilityChanged(bool show) {
  // Clear any existing mouse hover effects when the cursor becomes invisible.
  // Note we do not need to dispatch a mouse enter when the cursor becomes
  // visible because that can only happen in response to a mouse event, which
  // will trigger its own mouse enter.
  if (!show) {
    crui::EventDispatchDetails details = dispatcher()->DispatchMouseExitAtPoint(
        nullptr, dispatcher()->GetLastMouseLocationInRoot(),
        crui::EF_CURSOR_HIDE);
    if (details.dispatcher_destroyed)
      return;
  }

  OnCursorVisibilityChangedNative(show);
}

void WindowTreeHost::MoveCursorToLocationInDIP(
    const gfx::Point& location_in_dip) {
  gfx::Point host_location(location_in_dip);
  ConvertDIPToPixels(&host_location);
  MoveCursorToInternal(location_in_dip, host_location);
}

void WindowTreeHost::MoveCursorToLocationInPixels(
    const gfx::Point& location_in_pixels) {
  gfx::Point root_location(location_in_pixels);
  ConvertPixelsToDIP(&root_location);
  MoveCursorToInternal(root_location, location_in_pixels);
}

///ui::InputMethod* WindowTreeHost::GetInputMethod() {
///  if (!input_method_) {
///    input_method_ =
///        ui::CreateInputMethod(this, GetAcceleratedWidget()).release();
///    owned_input_method_ = true;
///  }
///  return input_method_;
///}

///void WindowTreeHost::SetSharedInputMethod(ui::InputMethod* input_method) {
///  if (input_method_ && owned_input_method_)
///    delete input_method_;
///  input_method_ = input_method;
///  owned_input_method_ = false;
///}

///crui::EventDispatchDetails WindowTreeHost::DispatchKeyEventPostIME(
///    crui::KeyEvent* event) {
///  // If dispatch to IME is already disabled we shouldn't reach here.
///  CR_DCHECK(!dispatcher_->should_skip_ime());
///  dispatcher_->set_skip_ime(true);
///
///  // InputMethod::DispatchKeyEvent() is called in PRE_DISPATCH phase, so event
///  // target is reset here to avoid issues in subsequent processing phases.
///  crui::Event::DispatcherApi(event).set_target(nullptr);
///
///  // We should bypass event rewriters here as they've been tried before.
///  crui::EventDispatchDetails dispatch_details =
///      event_sink()->OnEventFromSource(event);
///  if (!dispatch_details.dispatcher_destroyed)
///    dispatcher_->set_skip_ime(false);
///  return dispatch_details;
///}

crui::EventSink* WindowTreeHost::GetEventSink() {
  return dispatcher_.get();
}

int64_t WindowTreeHost::GetDisplayId() {
  return display::Screen::GetScreen()->GetDisplayNearestWindow(window()).id();
}

void WindowTreeHost::Show() {
  // Ensure that compositor has been properly initialized, see InitCompositor()
  // and InitHost().
  ///CR_DCHECK(compositor());
  ///CR_DCHECK(compositor()->root_layer() == window()->layer());
  ///compositor()->SetVisible(true);
  ShowImpl();
  window()->Show();
}

void WindowTreeHost::Hide() {
  HideImpl();
  /// if (compositor())
  ///  compositor()->SetVisible(false);
}

std::unique_ptr<ScopedKeyboardHook> WindowTreeHost::CaptureSystemKeyEvents(
    cr::Optional<cr::flat_set<crui::DomCode>> dom_codes) {
  // TODO(joedow): Remove the simple hook class/logic once this flag is removed.
  ///if (!cr::FeatureList::IsEnabled(features::kSystemKeyboardLock))
  ///  return std::make_unique<ScopedSimpleKeyboardHook>(std::move(dom_codes));

  if (CaptureSystemKeyEventsImpl(std::move(dom_codes)))
    return std::make_unique<ScopedKeyboardHook>(weak_factory_.GetWeakPtr());
  return nullptr;
}

bool WindowTreeHost::ShouldSendKeyEventToIme() {
  return true;
}

void WindowTreeHost::EnableNativeWindowOcclusionTracking() {
#if defined(MINI_CHROMIUM_OS_WIN)
  ///if (IsNativeWindowOcclusionEnabled()) {
  ///  NativeWindowOcclusionTrackerWin::GetOrCreateInstance()->Enable(window());
  ///}
#endif  // MINI_CHROMIUM_OS_WIN
}

void WindowTreeHost::DisableNativeWindowOcclusionTracking() {
#if defined(MINI_CHROMIUM_OS_WIN)
  ///if (IsNativeWindowOcclusionEnabled()) {
  ///  occlusion_state_ = Window::OcclusionState::UNKNOWN;
  ///  NativeWindowOcclusionTrackerWin::GetOrCreateInstance()->Disable(window());
  ///}
#endif  // MINI_CHROMIUM_OS_WIN
}

void WindowTreeHost::SetNativeWindowOcclusionState(
    Window::OcclusionState state) {
  if (occlusion_state_ != state) {
    occlusion_state_ = state;
    for (WindowTreeHostObserver& observer : observers_)
      observer.OnOcclusionStateChanged(this, state);
  }
}

std::unique_ptr<ScopedEnableUnadjustedMouseEvents>
WindowTreeHost::RequestUnadjustedMovement() {
  CR_NOTIMPLEMENTED();
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHost, protected:

WindowTreeHost::WindowTreeHost(std::unique_ptr<Window> window)
    : window_(window.release()),  // See header for details on ownership.
      occlusion_state_(Window::OcclusionState::UNKNOWN),
      last_cursor_(crui::CursorType::kNull),
      ///input_method_(nullptr),
      owned_input_method_(false) {
  if (!window_)
    window_ = new Window(nullptr);
  display::Screen::GetScreen()->AddObserver(this);
  auto display = display::Screen::GetScreen()->GetDisplayNearestWindow(window_);
  device_scale_factor_ = display.device_scale_factor();
}

void WindowTreeHost::IntializeDeviceScaleFactor(float device_scale_factor) {
  ///DCHECK(!compositor_->root_layer()) << "Only call this before InitHost()";
  device_scale_factor_ = device_scale_factor;
}

void WindowTreeHost::DestroyCompositor() {
  ///if (compositor_) {
  ///  compositor_->RemoveObserver(this);
  ///  compositor_.reset();
  ///}
}

void WindowTreeHost::DestroyDispatcher() {
  delete window_;
  window_ = nullptr;
  dispatcher_.reset();

  // TODO(beng): this comment is no longer quite valid since this function
  // isn't called from WED, and WED isn't a subclass of Window. So it seems
  // like we could just rely on ~Window now.
  // Destroy child windows while we're still valid. This is also done by
  // ~Window, but by that time any calls to virtual methods overriden here (such
  // as GetRootWindow()) result in Window's implementation. By destroying here
  // we ensure GetRootWindow() still returns this.
  //window()->RemoveOrDestroyChildren();
}

void WindowTreeHost::CreateCompositor(///const viz::FrameSinkId& frame_sink_id,
                                      bool force_software_compositor,
                                      bool use_external_begin_frame_control,
                                      const char* trace_environment_name) {
  ///Env* env = Env::GetInstance();
  ///crui::ContextFactory* context_factory = env->context_factory();
  ///CR_DCHECK(context_factory);
  ///crui::ContextFactoryPrivate* context_factory_private =
  ///    env->context_factory_private();
  ///compositor_ = std::make_unique<ui::Compositor>(
  ///    (!context_factory_private || frame_sink_id.is_valid())
  ///        ? frame_sink_id
  ///        : context_factory_private->AllocateFrameSinkId(),
  ///    context_factory, context_factory_private,
  ///    cr::ThreadTaskRunnerHandle::Get(), ui::IsPixelCanvasRecordingEnabled(),
  ///    use_external_begin_frame_control, force_software_compositor,
   ///   trace_environment_name);
  if (!dispatcher()) {
    window()->Init(/*crui::LAYER_NOT_DRAWN*/);
    window()->set_host(this);
    window()->SetName("RootWindow");
    dispatcher_ = std::make_unique<WindowEventDispatcher>(this);
  }
}

void WindowTreeHost::InitCompositor() {
  ///CR_DCHECK(!compositor_->root_layer());
  ///compositor_->SetScaleAndSize(device_scale_factor_, GetBoundsInPixels().size(),
  ///                             window()->GetLocalSurfaceIdAllocation());
  ///compositor_->SetRootLayer(window()->layer());
  ///
  ///display::Display display =
  ///    display::Screen::GetScreen()->GetDisplayNearestWindow(window());
  ///compositor_->SetDisplayColorSpaces(display.color_spaces());
}

void WindowTreeHost::OnAcceleratedWidgetAvailable() {
  ///compositor_->SetAcceleratedWidget(GetAcceleratedWidget());
  prop_ = std::make_unique<crui::ViewProp>(
      GetAcceleratedWidget(), kWindowTreeHostForAcceleratedWidget, this);
}

void WindowTreeHost::OnHostMovedInPixels(
    const gfx::Point& new_location_in_pixels) {
  ///TRACE_EVENT1("ui", "WindowTreeHost::OnHostMovedInPixels", "origin",
  ///             new_location_in_pixels.ToString());

  for (WindowTreeHostObserver& observer : observers_)
    observer.OnHostMovedInPixels(this, new_location_in_pixels);
}

void WindowTreeHost::OnHostResizedInPixels(
    const gfx::Size& new_size_in_pixels) {
  // The compositor is deleted from WM_DESTROY, but we don't delete things until
  // WM_NCDESTROY, and it must be possible to still get some messages between
  // these two.
  ///if (!compositor_)
  ///  return;
  ///
  ///display::Display display =
  ///    display::Screen::GetScreen()->GetDisplayNearestWindow(window());
  ///device_scale_factor_ = display.device_scale_factor();
  ///UpdateRootWindowSizeInPixels();
  ///
  ///// Passing |new_size_in_pixels| to set compositor size. It could be different
  ///// from GetBoundsInPixels() on Windows to contain extra space for window
  ///// transition animations and should be used to set compositor size instead of
  ///// GetBoundsInPixels() in such case.
  ///UpdateCompositorScaleAndSize(new_size_in_pixels);
  ///
  ///for (WindowTreeHostObserver& observer : observers_)
  ///  observer.OnHostResized(this);
}

void WindowTreeHost::OnHostWorkspaceChanged() {
  for (WindowTreeHostObserver& observer : observers_)
    observer.OnHostWorkspaceChanged(this);
}

void WindowTreeHost::OnHostDisplayChanged() {
  ///if (!compositor_)
  ///  return;
  ///display::Display display =
  ///    display::Screen::GetScreen()->GetDisplayNearestWindow(window());
  ///compositor_->SetDisplayColorSpaces(display.color_spaces());
}

void WindowTreeHost::OnHostCloseRequested() {
  for (WindowTreeHostObserver& observer : observers_)
    observer.OnHostCloseRequested(this);
}

void WindowTreeHost::OnHostLostWindowCapture() {
  // It is possible for this function to be called during destruction, after the
  // root window has already been destroyed (e.g. when the ui::PlatformWindow is
  // destroyed, and during destruction, it loses capture. See more details in
  // http://crbug.com/770670)
  if (!window())
    return;
  Window* capture_window = client::GetCaptureWindow(window());
  if (capture_window && capture_window->GetRootWindow() == window())
    capture_window->ReleaseCapture();
}

void WindowTreeHost::OnDisplayMetricsChanged(const display::Display& display,
                                             uint32_t metrics) {
  ///if (metrics & DisplayObserver::DISPLAY_METRIC_COLOR_SPACE) {
  ///  display::Screen* screen = display::Screen::GetScreen();
  ///  if (compositor_ &&
  ///      display.id() == screen->GetDisplayNearestView(window()).id()) {
  ///    compositor_->SetDisplayColorSpaces(display.color_spaces());
  ///  }
  ///}
}

gfx::Rect WindowTreeHost::GetTransformedRootWindowBoundsInPixels(
    const gfx::Size& size_in_pixels) const {
  gfx::RectF new_bounds = gfx::RectF(gfx::Rect(size_in_pixels));
  ///GetInverseRootTransform().TransformRect(&new_bounds);
  new_bounds = GetInverseRootTransform().MapRect(new_bounds);
  return gfx::ToEnclosingRect(new_bounds);
}

////////////////////////////////////////////////////////////////////////////////
// WindowTreeHost, private:

void WindowTreeHost::MoveCursorToInternal(const gfx::Point& root_location,
                                          const gfx::Point& host_location) {
  last_cursor_request_position_in_host_ = host_location;
  MoveCursorToScreenLocationInPixels(host_location);
  client::CursorClient* cursor_client = client::GetCursorClient(window());
  if (cursor_client) {
    const display::Display& display =
        display::Screen::GetScreen()->GetDisplayNearestWindow(window());
    cursor_client->SetDisplay(display);
  }
  dispatcher()->OnCursorMovedToRootLocation(root_location);
}

///void WindowTreeHost::OnCompositingEnded(crui::Compositor* compositor) {
///  if (!holding_pointer_moves_)
///    return;
///
///  dispatcher_->ReleasePointerMoves();
///  holding_pointer_moves_ = false;
///  DCHECK(!synchronization_start_time_.is_null());
///  UMA_HISTOGRAM_TIMES("UI.WindowTreeHost.SurfaceSynchronizationDuration",
///                      base::TimeTicks::Now() - synchronization_start_time_);
///}
///
///void WindowTreeHost::OnCompositingChildResizing(ui::Compositor* compositor) {
///  if (!Env::GetInstance()->throttle_input_on_resize() || holding_pointer_moves_)
///    return;
///  synchronization_start_time_ = base::TimeTicks::Now();
///  dispatcher_->HoldPointerMoves();
///  holding_pointer_moves_ = true;
///}
///
///void WindowTreeHost::OnCompositingShuttingDown(ui::Compositor* compositor) {
///  compositor->RemoveObserver(this);
///}

}  // namespace aura
}  // namespace crui
