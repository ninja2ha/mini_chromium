#include <iostream>
#include <string>

#include "crbase/strings/string_number_conversions.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/memory/weak_ptr.h"
#include "crbase/bits.h"

#include "crbase/functional/callback.h"
#include "crbase/functional/bind.h"

using namespace std;

//------------------------------------------------------------------------------

namespace {
 
class MyClass : public cr::RefCounted<MyClass> {
 public:
  CR_REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE;

  MyClass() { std::cout << HasOneRef() << std::endl;}
  ~MyClass() { std::cout << "~MyClass" << std::endl; }

  void DoPrint() { std::cout << "doPrINT" << endl; }
  cr::WeakPtr<MyClass> as_weak() { return weak_ptr_fac_.GetWeakPtr(); }

 private:
  friend class cr::RefCounted<MyClass>;

  cr::WeakPtrFactory<MyClass> weak_ptr_fac_{ this };
};

}  // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv) {
  cr::RefPtr<MyClass> myclass = cr::MakeRefCounted<MyClass>();

  std::cout << myclass->HasAtLeastOneRef() << std::endl;

  double d1;
  cr::StringToDouble("1.561", &d1);
  std::cout << d1 << endl;

  int i1;
  cr::StringToInt("-124", &i1);
  std::cout << i1 << endl;

  uint32_t ui1;
  cr::StringToUint("-124", &ui1);
  std::cout << ui1 << endl;

  int64_t i2;
  cr::StringToInt64("123456789101", &i2);
  std::cout << i2 << endl;

  cr::HexStringToInt64("0x00007F0080000000", &i2);
  std::cout << i2 << endl;

  int64_t i3;
  cr::HexStringToInt64("00007F0080000000", &i3);
  std::cout << i2 << endl;

  int m = 63;
  m = cr::bits::AlignUp(m, 64);

  cr::OnceClosure cb = cr::BindOnce(&MyClass::DoPrint, myclass->as_weak());
  std::move(cb).Run();
  return 0;
}

//------------------------------------------------------------------------------

//
// Outputs:
//
// hello world!, 1, 2, 124.262
// 1234,                mmhelo, s
//
