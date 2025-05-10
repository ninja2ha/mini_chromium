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

bool SimpleUIApplication::CanResize() const { return false; }
bool SimpleUIApplication::CanMaximize() const { return false; }
bool SimpleUIApplication::CanMinimize() const { return true; }

cr::string16 SimpleUIApplication::GetWindowTitle() const {
  return cr::ASCIIToUTF16("SimpleWidget");
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
  using namespace crui;
  MainThreadHelper main_thread(cr::MessageLoop::TYPE_UI, true);

  views::Widget* widget = new views::Widget;
  views::Widget::InitParams params;
  params.delegate = new SimpleUIApplication(widget);
  params.context = nullptr;
  params.force_software_compositing = true;
  widget->Init(std::move(params));
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