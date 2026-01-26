// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <utility>

#include "crbase/win/com/scoped_variant.h"
#include "crbase/win/com/scoped_comptr.h"

#include "gtest/gtest.h"

namespace cr {
namespace win {

namespace {

constexpr wchar_t kTestString[] = L"Test string for BSTRs.";

void InitializeVariantWithBstr(VARIANT* var) {
  if (!var) {
    ADD_FAILURE() << "|var| cannot be null.";
    return;
  }

  var->vt = VT_BSTR;
  V_BSTR(var) = ::SysAllocString(kTestString);
}

void ExpectRefCount(ULONG expected_refcount, IUnknown* object) {
  // In general, code should not check the values of AddRef() and Release().
  // However, tests need to validate that ScopedVariant safely owns a COM object
  // so they are checked for this unit test.
  EXPECT_EQ(expected_refcount + 1, object->AddRef());
  EXPECT_EQ(expected_refcount, object->Release());
}

void ExpectVariantType(VARENUM var_type, const ScopedVariant& var) {
  EXPECT_EQ(var_type, var.type());
  EXPECT_EQ(var_type, V_VT(var.ptr()));
}


// A dummy IDispatch implementation (if you can call it that).
// The class does nothing intelligent really.  Only increments a counter
// when AddRef is called and decrements it when Release is called.
class DispatchStub : public IDispatch {
 public:
  DispatchStub() : ref_(0) {
  }

  STDMETHOD_(DWORD, AddRef)() override {
    ref_++;
    return ref_;
  }

  STDMETHOD_(DWORD, Release)() override {
    ref_--;
    return ref_;
  }

  STDMETHOD(QueryInterface)(REFIID, void**) override { return E_NOTIMPL; }

  STDMETHOD(GetTypeInfoCount)(UINT*) override { return E_NOTIMPL; }

  STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) override { return E_NOTIMPL; }

  STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override {
    return E_NOTIMPL;
  }

  STDMETHOD(Invoke)(DISPID,
                    REFIID,
                    LCID,
                    WORD,
                    DISPPARAMS*,
                    VARIANT*,
                    EXCEPINFO*,
                    UINT*) override {
    return E_NOTIMPL;
  }

  // A way to check the internal reference count of the class.
  int ref_count() const {
    return ref_;
  }

 protected:
  int ref_;
};

}  // namespace

TEST(ScopedVariantTest, Empty) {
  ScopedVariant var;
  ExpectVariantType(VT_EMPTY, var);
}

TEST(ScopedVariantTest, ConstructBstr) {
  ScopedVariant var(kTestString);
  ExpectVariantType(VT_BSTR, var);
  EXPECT_STREQ(kTestString, V_BSTR(var.ptr()));
}

TEST(ScopedVariantTest, SetBstr) {
  ScopedVariant var;
  var.Set(kTestString);
  ExpectVariantType(VT_BSTR, var);
  EXPECT_STREQ(kTestString, V_BSTR(var.ptr()));
}

TEST(ScopedVariantTest, ReleaseBstr) {
  ScopedVariant var;
  var.Set(kTestString);
  VARIANT released_variant = var.Release();
  ExpectVariantType(VT_EMPTY, var);
  EXPECT_EQ(VT_BSTR, V_VT(&released_variant));
  EXPECT_STREQ(kTestString, V_BSTR(&released_variant));
  ::VariantClear(&released_variant);
}

TEST(ScopedVariantTest, ResetToEmptyBstr) {
  ScopedVariant var(kTestString);
  ExpectVariantType(VT_BSTR, var);
  var.Reset();
  ExpectVariantType(VT_EMPTY, var);
}

TEST(ScopedVariantTest, TakeOwnershipBstr) {
  VARIANT bstr_variant;
  bstr_variant.vt = VT_BSTR;
  bstr_variant.bstrVal = ::SysAllocString(kTestString);

  ScopedVariant var;
  var.Reset(bstr_variant);
  ExpectVariantType(VT_BSTR, var);
  EXPECT_EQ(bstr_variant.bstrVal, V_BSTR(var.ptr()));
}

TEST(ScopedVariantTest, SwapBstr) {
  ScopedVariant from(kTestString);
  ScopedVariant to;
  to.Swap(from);
  ExpectVariantType(VT_EMPTY, from);
  ExpectVariantType(VT_BSTR, to);
  EXPECT_STREQ(kTestString, V_BSTR(to.ptr()));
}

TEST(ScopedVariantTest, CompareBstr) {
  ScopedVariant var_bstr1;
  InitializeVariantWithBstr(var_bstr1.Receive());
  ScopedVariant var_bstr2(V_BSTR(var_bstr1.ptr()));
  EXPECT_EQ(0, var_bstr1.Compare(var_bstr2));

  var_bstr2.Reset();
  EXPECT_NE(0, var_bstr1.Compare(var_bstr2));
}

TEST(ScopedVariantTest, ReceiveAndCopyBstr) {
  ScopedVariant var_bstr1;
  InitializeVariantWithBstr(var_bstr1.Receive());
  ScopedVariant var_bstr2;
  var_bstr2.Reset(var_bstr1.Copy());
  EXPECT_EQ(0, var_bstr1.Compare(var_bstr2));
}

TEST(ScopedVariantTest, SetBstrFromBstrVariant) {
  ScopedVariant var_bstr1;
  InitializeVariantWithBstr(var_bstr1.Receive());
  ScopedVariant var_bstr2;
  var_bstr2.Set(V_BSTR(var_bstr1.ptr()));
  EXPECT_EQ(0, var_bstr1.Compare(var_bstr2));
}

TEST(ScopedVariantTest, SetDate) {
  ScopedVariant var;
  SYSTEMTIME sys_time;
  ::GetSystemTime(&sys_time);
  DATE date;
  ::SystemTimeToVariantTime(&sys_time, &date);
  var.SetDate(date);
  ExpectVariantType(VT_DATE, var);
  EXPECT_EQ(date, V_DATE(var.ptr()));
}

TEST(ScopedVariantTest, SetSigned1Byte) {
  ScopedVariant var;
  var.Set(static_cast<int8_t>('v'));
  ExpectVariantType(VT_I1, var);
  EXPECT_EQ('v', V_I1(var.ptr()));
}

TEST(ScopedVariantTest, SetSigned2Byte) {
  ScopedVariant var;
  var.Set(static_cast<short>(123));
  ExpectVariantType(VT_I2, var);
  EXPECT_EQ(123, V_I2(var.ptr()));
}

TEST(ScopedVariantTest, SetSigned4Byte) {
  ScopedVariant var;
  var.Set(123);
  ExpectVariantType(VT_I4, var);
  EXPECT_EQ(123, V_I4(var.ptr()));
}

TEST(ScopedVariantTest, SetSigned8Byte) {
  ScopedVariant var;
  var.Set(static_cast<int64_t>(123));
  ExpectVariantType(VT_I8, var);
  EXPECT_EQ(123, V_I8(var.ptr()));
}

TEST(ScopedVariantTest, SetUnsigned1Byte) {
  ScopedVariant var;
  var.Set(static_cast<uint8_t>(123));
  ExpectVariantType(VT_UI1, var);
  EXPECT_EQ(123u, V_UI1(var.ptr()));
}

TEST(ScopedVariantTest, SetUnsigned2Byte) {
  ScopedVariant var;
  var.Set(static_cast<unsigned short>(123));
  ExpectVariantType(VT_UI2, var);
  EXPECT_EQ(123u, V_UI2(var.ptr()));
}

TEST(ScopedVariantTest, SetUnsigned4Byte) {
  ScopedVariant var;
  var.Set(static_cast<uint32_t>(123));
  ExpectVariantType(VT_UI4, var);
  EXPECT_EQ(123u, V_UI4(var.ptr()));
}

TEST(ScopedVariantTest, SetUnsigned8Byte) {
  ScopedVariant var;
  var.Set(static_cast<uint64_t>(123));
  ExpectVariantType(VT_UI8, var);
  EXPECT_EQ(123u, V_UI8(var.ptr()));
}

TEST(ScopedVariantTest, SetReal4Byte) {
  ScopedVariant var;
  var.Set(123.123f);
  ExpectVariantType(VT_R4, var);
  EXPECT_EQ(123.123f, V_R4(var.ptr()));
}

TEST(ScopedVariantTest, SetReal8Byte) {
  ScopedVariant var;
  var.Set(static_cast<double>(123.123));
  ExpectVariantType(VT_R8, var);
  EXPECT_EQ(123.123, V_R8(var.ptr()));
}

TEST(ScopedVariantTest, SetBooleanTrue) {
  ScopedVariant var;
  var.Set(true);
  ExpectVariantType(VT_BOOL, var);
  EXPECT_EQ(VARIANT_TRUE, V_BOOL(var.ptr()));
}

TEST(ScopedVariantTest, SetBooleanFalse) {
  ScopedVariant var;
  var.Set(false);
  ExpectVariantType(VT_BOOL, var);
  EXPECT_EQ(VARIANT_FALSE, V_BOOL(var.ptr()));
}

TEST(ScopedVariantTest, SetComIDispatch) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IDispatch) dispatch_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));

  ScopedVariant var;
  ExpectRefCount(1U, dispatch_stub.get());
  var.Set(dispatch_stub.get());
  ExpectVariantType(VT_DISPATCH, var);
  EXPECT_EQ(dispatch_stub.get(), V_DISPATCH(var.ptr()));
  ExpectRefCount(2U, dispatch_stub.get());
  var.Reset();
  ExpectRefCount(1U, dispatch_stub.get());
}

TEST(ScopedVariantTest, SetComNullIDispatch) {
  ScopedVariant var;
  var.Set(static_cast<IDispatch*>(nullptr));
  ExpectVariantType(VT_DISPATCH, var);
  EXPECT_EQ(nullptr, V_DISPATCH(var.ptr()));
}

TEST(ScopedVariantTest, SetComIUnknown) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IUnknown) unknown_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));

  ScopedVariant var;
  ExpectRefCount(1U, unknown_stub.get());
  var.Set(unknown_stub.get());
  ExpectVariantType(VT_UNKNOWN, var);
  EXPECT_EQ(unknown_stub.get(), V_UNKNOWN(var.ptr()));
  ExpectRefCount(2U, unknown_stub.get());
  var.Reset();
  ExpectRefCount(1U, unknown_stub.get());
}

TEST(ScopedVariantTest, SetComNullIUnknown) {
  ScopedVariant var;
  var.Set(static_cast<IUnknown*>(nullptr));
  ExpectVariantType(VT_UNKNOWN, var);
  EXPECT_EQ(nullptr, V_UNKNOWN(var.ptr()));
}

TEST(ScopedVariant, ScopedComIDispatchConstructor) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IDispatch) dispatch_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));
  {
    ScopedVariant var(dispatch_stub.get());
    ExpectVariantType(VT_DISPATCH, var);
    EXPECT_EQ(dispatch_stub.get(), V_DISPATCH(var.ptr()));
    ExpectRefCount(2U, dispatch_stub.get());
  }
  ExpectRefCount(1U, dispatch_stub.get());
}

TEST(ScopedVariant, ScopedComIDispatchMove) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IDispatch) dispatch_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));
  {
    ScopedVariant var1(dispatch_stub.get());
    ExpectRefCount(2U, dispatch_stub.get());
    ScopedVariant var2(std::move(var1));
    ExpectRefCount(2U, dispatch_stub.get());
    ScopedVariant var3;
    var3 = std::move(var2);
    ExpectRefCount(2U, dispatch_stub.get());
  }
  ExpectRefCount(1U, dispatch_stub.get());
}

TEST(ScopedVariant, ScopedComIDispatchCopy) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IDispatch) dispatch_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));
  {
    ScopedVariant var1(dispatch_stub.get());
    ExpectRefCount(2U, dispatch_stub.get());
    ScopedVariant var2(static_cast<const VARIANT&>(var1));
    ExpectRefCount(3U, dispatch_stub.get());
    ScopedVariant var3;
    var3 = static_cast<const VARIANT&>(var2);
    ExpectRefCount(4U, dispatch_stub.get());
  }
  ExpectRefCount(1U, dispatch_stub.get());
}

TEST(ScopedVariant, ScopedComIUnknownConstructor) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IUnknown) unknown_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));
  {
    ScopedVariant unk_var(unknown_stub.get());
    ExpectVariantType(VT_UNKNOWN, unk_var);
    EXPECT_EQ(unknown_stub.get(), V_UNKNOWN(unk_var.ptr()));
    ExpectRefCount(2U, unknown_stub.get());
  }
  ExpectRefCount(1U, unknown_stub.get());
}

TEST(ScopedVariant, ScopedComIUnknownWithRawVariant) {
  DispatchStub dispatch_stub_obj;
  CR_SCOPED_COMPTR(IUnknown) unknown_stub(
      static_cast<IDispatch*>(&dispatch_stub_obj));
  ScopedVariant var;
  VARIANT raw;
  raw.vt = VT_UNKNOWN;
  raw.punkVal = unknown_stub.get();
  ExpectRefCount(1U, unknown_stub.get());
  var.Set(raw);
  ExpectRefCount(2U, unknown_stub.get());
  var.Reset();
  ExpectRefCount(1U, unknown_stub.get());
}

TEST(ScopedVariant, SetSafeArray) {
  SAFEARRAY* sa = ::SafeArrayCreateVector(VT_UI1, 0, 100);
  ASSERT_TRUE(sa);

  ScopedVariant var;
  var.Set(sa);
  EXPECT_TRUE(ScopedVariant::IsLeakableVarType(var.type()));
  ExpectVariantType(static_cast<VARENUM>(VT_ARRAY | VT_UI1), var);
  EXPECT_EQ(sa, V_ARRAY(var.ptr()));
  // The array is destroyed in the destructor of var.
  sa = nullptr;
}

TEST(ScopedVariant, SetNullSafeArray) {
  ScopedVariant var;
  var.Set(static_cast<SAFEARRAY*>(nullptr));
  ExpectVariantType(VT_EMPTY, var);
}

}  // namespace win
}  // namespace cr