#include <iostream>
#include <string>

#include "crbase/containers/optional.h"

using namespace std;

//------------------------------------------------------------------------------

namespace {
 
// optional can be used as the return type of a factory that may fail
cr::Optional<std::string> create(bool b) {
  if (b)
    return "Godzilla";
  return {};
}

// std::nullopt can be used to create any (empty) std::optional
cr::Optional<std::string> create2(bool b) {
  return b ? cr::Optional<std::string>("Godzilla") : cr::nullopt;
}

}  // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv) {
  std::cout << "create(false) returned "
    << create(false).value_or("empty") << '\n';

  // optional-returning factory functions are usable as conditions of while and
  // if
  if (cr::Optional<std::string> str = create2(true))
    std::cout << "create2(true) returned " << *str << '\n';

  // won`t print.
  if (cr::Optional<std::string> str = create2(false))
    std::cout << "create2(false) returned " << *str << '\n';

  return 0;
}

//------------------------------------------------------------------------------

//
// Outputs:
//
// create(false) returned empty
// create2(true) returned Godzilla
//