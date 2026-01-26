#include <stdio.h>
#include <iostream>

#include "crbase/logging/logging.h"
#include "crbase/stl_util.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_util.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback.h"
#include "crbase/at_exit.h"
#include "crbase/numerics/safe_math.h"
#include "crbase/time/time.h"
#include "crbase/time/time_to_iso8601.h"
#include "crbase/memory/singleton.h"
#include "crbase/win/windows_types.h"

#pragma comment(lib, "winmm")

int add(int a, int b) {
  return a + b;
}

int main(int argc, char*  argv[]) {
  cr::AtExitManager at_exit_manager;

  constexpr cr::StringPiece hello = "hello world!!";
  constexpr char font_char = hello.front();
  
  int a = 500;
  int b = 500;
  auto sum = cr::CheckAdd(a, b);
  auto sum2 = sum.ValueOrDie();

  cr::Time now = cr::Time::Now();
  std::string now_str = cr::TimeToISO8601(now);
  printf("now_str:%s\n", now_str.c_str());

  system("pause");
  return 0;
}
