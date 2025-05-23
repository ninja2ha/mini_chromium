#include "crbase/functional/bind.h"
#include "crbase/at_exit.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/message_loop/message_loop.h"
#include "crbase/win/msvc_import_libs.h"

#include "crui/views/widget/widget.h"
#include "crui/views/widget/widget_delegate.h"
#include "crui/views/views_delegate.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_USE_AURA)
#include "crui/aura/env.h"
#include "crui/aura/wm/core/wm_state.h"
#endif

#if defined(MINI_CHROMIUM_ENABLE_DESKTOP_AURA)
#include "crui/display/screen.h"
#include "crui/display/display.h"
#include "crui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "crui/views/widget/desktop_aura/desktop_screen.h"
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crui/base/win/scoped_ole_initializer.h"
#endif

#include "examples/common/logging_initializtion.h"
#include "examples/common/thread_helper.h"

////////////////////////////////////////////////////////////////////////////////

namespace example {

class SimpleUIApplication : public crui::views::WidgetDelegate,
                            public crui::views::ViewsDelegate {
 public:
  SimpleUIApplication(const SimpleUIApplication&) = delete;
  SimpleUIApplication& operator=(const SimpleUIApplication&) = delete;

  SimpleUIApplication(const crui::views::Widget* widget);
  virtual ~SimpleUIApplication();
 
 protected:
  // crui::views::WidgetDelegate implements.
  bool CanResize() const override;
  bool CanMaximize() const override;
  bool CanMinimize() const override;
  cr::string16 GetWindowTitle() const override;
  void WindowClosing() override;
  void DeleteDelegate() override;

  // WidgetGetter implements
  const crui::views::Widget* GetWidgetImpl() const override;

 private:
  const crui::views::Widget* const widget_ = nullptr;
};

SimpleUIApplication::SimpleUIApplication(const crui::views::Widget* widget)
    : widget_(widget) {
}

SimpleUIApplication::~SimpleUIApplication() {
}

bool SimpleUIApplication::CanResize() const { return true; }
bool SimpleUIApplication::CanMaximize() const { return true; }
bool SimpleUIApplication::CanMinimize() const { return true; }

cr::string16 SimpleUIApplication::GetWindowTitle() const {
  return cr::ASCIIToUTF16("curi-demo");
}

void SimpleUIApplication::WindowClosing() {
  MainThreadHelper::Get()->AsyncQuit();
}

void SimpleUIApplication::DeleteDelegate() {
  delete this;
}

const crui::views::Widget* SimpleUIApplication::GetWidgetImpl() const {
  return widget_;
}

int RunSimpleWidget() {
#if defined(MINI_CHROMIUM_OS_WIN)
  crui::ScopedOleInitializer ole_initializer;
#endif

  using namespace crui;
  MainThreadHelper main_thread(cr::MessageLoop::TYPE_UI, true);

#if defined(MINI_CHROMIUM_USE_AURA)
  std::unique_ptr<aura::Env> env = aura::Env::CreateInstance();
  ///aura::Env::GetInstance()->set_context_factory(context_factory.get());
  ///aura::Env::GetInstance()->set_context_factory_private(context_factory.get());
  crui::wm::WMState wm_state;
#endif

#if defined(MINI_CHROMIUM_ENABLE_DESKTOP_AURA)
  std::unique_ptr<crui::display::Screen> desktop_screen(
      crui::views::CreateDesktopScreen());
  crui::display::Screen::SetScreenInstance(desktop_screen.get());
#endif

  views::Widget* widget = new views::Widget;
  views::Widget::InitParams params;
  params.delegate = new SimpleUIApplication(widget);
#if defined(MINI_CHROMIUM_ENABLE_DESKTOP_AURA)
  params.native_widget = new crui::views::DesktopNativeWidgetAura(widget);
#endif
  params.force_software_compositing = true;
  params.bounds = crui::gfx::Rect(800, 600);
  widget->Init(std::move(params));
  widget->Show();

  main_thread.Run();

#if defined(MINI_CHROMIUM_USE_AURA)
  env.reset();
#endif
  return 0;
}

}  // namesapce

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv) {
  example::InitLogging();

  cr::AtExitManager at_exit;
  return example::RunSimpleWidget();
}