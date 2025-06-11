#include "crbase/functional/bind.h"
#include "crbase/at_exit.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/message_loop/message_loop.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/win/msvc_import_libs.h"

#include "crui/display/screen.h"
#include "crui/display/win/screen_win.h"
#include "crui/views/widget/widget.h"
#include "crui/views/widget/widget_delegate.h"
#include "crui/views/views_delegate.h"
#include "crui/views/view_background.h"
#include "crui/views/layout/box_layout.h"
#include "crui/views/layout/grid_layout.h"
#include "crui/views/controls/button/button.h"
#include "crui/base/build_platform.h"

#include "examples/common/logging_initializtion.h"
#include "examples/common/thread_helper.h"

////////////////////////////////////////////////////////////////////////////////

namespace example {

class SimpleUIApplication : public crui::views::WidgetDelegateView,
                            public crui::views::ButtonListener,
                            public crui::views::ViewsDelegate {
 public:
  SimpleUIApplication(const SimpleUIApplication&) = delete;
  SimpleUIApplication& operator=(const SimpleUIApplication&) = delete;

  SimpleUIApplication();
  virtual ~SimpleUIApplication();
 
 protected:
  // crui::views::WidgetDelegate implements.
  bool CanResize() const override;
  bool CanMaximize() const override;
  bool CanMinimize() const override;
  cr::string16 GetWindowTitle() const override;
  void WindowClosing() override;

  //
  void ButtonPressed(crui::views::Button* sender, const crui::Event& event);

};

SimpleUIApplication::SimpleUIApplication() {
  using namespace crui::views;
  this->SetBackground(CreateSolidBackground(SK_ColorWHITE));

  Button* button = new Button(this);
  button->SetSize(crui::gfx::Size(120, 30));
  button->SetPosition(crui::gfx::Point(300, 400));
  button->SetFocusPainter(
      Painter::CreateSolidFocusPainter(
          SK_ColorBLUE, crui::gfx::Insets(1)));
  button->SetBackground(CreateRoundedRectBackground(SK_ColorCYAN, 5));
  this->AddChildView(button);
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

void SimpleUIApplication::ButtonPressed(crui::views::Button* sender, 
                                        const crui::Event& event) {
  CR_LOG(Info) << "button(" << sender->GetID() << ") pressed";
}

int RunSimpleWidget() {
  using namespace crui;
  MainThreadHelper main_thread(cr::MessageLoop::TYPE_UI, true);

  views::Widget* widget = new views::Widget;
  views::Widget::InitParams params;
  params.delegate = new SimpleUIApplication();
  params.context = nullptr;
  params.force_software_compositing = true;
  params.bounds = crui::gfx::Rect(800, 600);
  ///params.mirror_origin_in_rtl = true;
  params.opacity = crui::views::Widget::InitParams::WindowOpacity::kTranslucent;
  widget->Init(std::move(params));
  widget->CenterWindow(params.bounds.size());
  widget->Show();

  main_thread.Run();
  return 0;
}

}  // namesapce

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv) {
  example::InitLogging();

  cr::AtExitManager at_exit;
  return example::RunSimpleWidget();
}