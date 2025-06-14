
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGParser_DEFINED
#define SkSVGParser_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkTDict.h"
#include "include/svg/parser/SkSVGPaintState.h"
#include "include/svg/parser/SkSVGTypes.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/xml/SkXMLParser.h"
#include "include/xml/SkXMLWriter.h"

class SkSVGBase;
class SkSVGElement;

class SkSVGParser : public SkXMLParser {
public:
    SkSVGParser(SkXMLParserError* err = NULL);
    virtual ~SkSVGParser();
    void _addAttribute(const char* attrName, const char* attrValue) {
        fXMLWriter.addAttribute(attrName, attrValue); }
    void _addAttribute(const char* attrName, SkString& attrValue) {
        fXMLWriter.addAttribute(attrName, attrValue.c_str()); }
    void _addAttributeLen(const char* attrName, const char* attrValue, size_t len) {
        fXMLWriter.addAttributeLen(attrName, attrValue, len); }
    void _endElement() { fXMLWriter.endElement(); }
    int findAttribute(SkSVGBase* , const char* attrValue, size_t len, bool isPaint);
//    const char* getFinal();
    SkTDict<SkSVGElement*>& getIDs() { return fIDs; }
    SkString& getPaintLast(SkSVGPaint::Field field);
    void _startElement(const char name[]) { fXMLWriter.startElement(name); }
    void translate(SkSVGElement*, bool isDef);
    void translateMatrix(SkString& , SkString* id);
    static void ConvertToArray(SkString& vals);
protected:
    virtual bool onAddAttribute(const char name[], const char value[]);
    bool onAddAttributeLen(const char name[], const char value[], size_t len);
    virtual bool onEndElement(const char elem[]);
    virtual bool onStartElement(const char elem[]);
    bool onStartElementLen(const char elem[], size_t len);
    virtual bool onText(const char text[], int len);
private:
    bool isStrokeAndFill(SkSVGPaint** stroke, SkSVGPaint** fill);
    static SkSVGElement* CreateElement(SkSVGTypes type, SkSVGElement* parent);
    static void Delete(SkTDArray<SkSVGElement*>& fChildren);
    static SkSVGTypes GetType(const char name[], size_t len);
    SkSVGPaint* fHead;
    SkSVGPaint fEmptyPaint;
    SkSVGPaint fLastFlush;
    SkString fLastColor;
    SkMatrix fLastTransform;
    SkTDArray<SkSVGElement*> fChildren;
    SkTDict<SkSVGElement*> fIDs;
    SkTDArray<SkSVGElement*> fParents;
    SkDynamicMemoryWStream fStream;
    SkXMLStreamWriter fXMLWriter;
    SkSVGElement*   fCurrElement;
    SkBool8 fInSVG;
    SkBool8 fSuppressPaint;
    friend class SkSVGPaint;
    friend class SkSVGGradient;
};

#endif // SkSVGParser_DEFINED
