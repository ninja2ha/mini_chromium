#include <stdio.h>
#include <iostream>

#include "cr_base/logging/logging.h"
#include "cr_base/stl_util.h"
#include "cr_base/strings/string_piece.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/memory/ref_counted.h"
#include "cr_base/functional/bind.h"
#include "cr_base/functional/callback.h"
#include "cr_base/at_exit.h"
#include "cr_base/numerics/safe_math.h"
#include "cr_base/time/time.h"
#include "cr_base/time/time_to_iso8601.h"
#include "cr_base/memory/singleton.h"
#include "cr_base/win/windows_types.h"

//#pragma comment(lib, "winmm")
//
//int add(int a, int b) {
//  return a + b;
//}
//
//int main(int argc, char*  argv[]) {
//  cr::AtExitManager at_exit_manager;
//
//  constexpr cr::StringPiece hello = "hello world!!";
//  constexpr char font_char = hello.front();
//  
//  int a = 500;
//  int b = 500;
//  auto sum = cr::CheckAdd(a, b);
//  auto sum2 = sum.ValueOrDie();
//
//  cr::Time now = cr::Time::Now();
//  std::string now_str = cr::TimeToISO8601(now);
//  printf("now_str:%s\n", now_str.c_str());
//
//  system("pause");
//  return 0;
//}
