
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpArray_DEFINED
#define SkOpArray_DEFINED

#include "src/animator/SkOperand2.h"
#include "src/animator/SkTDArray_Experimental.h"

typedef SkLongArray(SkOperand2) SkTDOperand2Array;

class SkOpArray : public SkTDOperand2Array {
public:
    SkOpArray();
    SkOpArray(SkOperand2::OpType type);
    bool getIndex(int index, SkOperand2* operand);
    SkOperand2::OpType getType() { return fType; }
    void setType(SkOperand2::OpType type) {
        fType = type;
    }
protected:
    SkOperand2::OpType fType;
};

#endif // SkOpArray_DEFINED
