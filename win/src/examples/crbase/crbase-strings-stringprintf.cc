#include <iostream>
#include <string>

#include "crbase/strings/stringprintf.h"

using namespace std;

//------------------------------------------------------------------------------

namespace {
 

}  // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv) {
  std::string r = cr::StringPrintf(
      "%s, %d, %u, %.03f", "hello world!", 1, 2, 124.262f);
  std::cout << r << endl;

  std::string r2;
  cr::SStringPrintf(&r2, "%3d, %21s, %c", 1234, "mmhelo", 's');
  std::cout << r2 << endl;
  return 0;
}

//------------------------------------------------------------------------------

//
// Outputs:
//
// hello world!, 1, 2, 124.262
// 1234,                mmhelo, s
//
