/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/views/SkWindow.h"
#include "include/core/SkCanvas.h"
#include "include/views/SkOSMenu.h"
#include "include/core/SkSurface.h"
#include "include/views/SkSystemEventTypes.h"
#include "include/core/SkTime.h"

#define SK_EventDelayInval "\xd" "n" "\xa" "l"

SkWindow::SkWindow()
    : fSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType)
    , fFocusView(nullptr)
{
    fClicks.reset();
    fWaitingOnInval = false;
    fColorType = kN32_SkColorType;
    fMatrix.reset();
}

SkWindow::~SkWindow() {
    fClicks.deleteAll();
    fMenus.deleteAll();
}

SkSurface* SkWindow::createSurface() {
    const SkBitmap& bm = this->getBitmap();
    return SkSurface::NewRasterDirect(bm.info(), bm.getPixels(), bm.rowBytes(), &fSurfaceProps);
}

void SkWindow::setMatrix(const SkMatrix& matrix) {
    if (fMatrix != matrix) {
        fMatrix = matrix;
        this->inval(nullptr);
    }
}

void SkWindow::preConcat(const SkMatrix& matrix) {
    SkMatrix m;
    m.setConcat(fMatrix, matrix);
    this->setMatrix(m);
}

void SkWindow::postConcat(const SkMatrix& matrix) {
    SkMatrix m;
    m.setConcat(matrix, fMatrix);
    this->setMatrix(m);
}

void SkWindow::setColorType(SkColorType ct) {
    this->resize(fBitmap.width(), fBitmap.height(), ct);
}

void SkWindow::resize(int width, int height, SkColorType ct) {
    if (ct == kUnknown_SkColorType)
        ct = fColorType;

    if (width != fBitmap.width() || height != fBitmap.height() || ct != fColorType) {
        fColorType = ct;
        fBitmap.allocPixels(SkImageInfo::Make(width, height,
                                              ct, kPremul_SkAlphaType));

        this->setSize(SkIntToScalar(width), SkIntToScalar(height));
        this->inval(nullptr);
    }
}

bool SkWindow::handleInval(const SkRect* localR) {
    SkIRect ir;

    if (localR) {
        SkRect devR;
        SkMatrix inverse;
        if (!fMatrix.invert(&inverse)) {
            return false;
        }
        fMatrix.mapRect(&devR, *localR);
        devR.round(&ir);
    } else {
        ir.set(0, 0,
               SkScalarRoundToInt(this->width()),
               SkScalarRoundToInt(this->height()));
    }
    fDirtyRgn.op(ir, SkRegion::kUnion_Op);

    this->onHandleInval(ir);
    return true;
}

void SkWindow::forceInvalAll() {
    fDirtyRgn.setRect(0, 0,
                      SkScalarCeilToInt(this->width()),
                      SkScalarCeilToInt(this->height()));
}

#ifdef SK_SIMULATE_FAILED_MALLOC
extern bool gEnableControlledThrow;
#endif

bool SkWindow::update(SkIRect* updateArea) {
    if (!fDirtyRgn.isEmpty()) {
        SkAutoTUnref<SkSurface> surface(this->createSurface());
        SkCanvas* canvas = surface->getCanvas();

        canvas->clipRegion(fDirtyRgn);
        if (updateArea) {
            *updateArea = fDirtyRgn.getBounds();
        }

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(fMatrix);

        // empty this now, so we can correctly record any inval calls that
        // might be made during the draw call.
        fDirtyRgn.setEmpty();

#ifdef SK_SIMULATE_FAILED_MALLOC
        gEnableControlledThrow = true;
#endif
#ifdef SK_BUILD_FOR_WIN32
        //try {
            this->draw(canvas);
        //}
        //catch (...) {
        //}
#else
        this->draw(canvas);
#endif
#ifdef SK_SIMULATE_FAILED_MALLOC
        gEnableControlledThrow = false;
#endif

        return true;
    }
    return false;
}

bool SkWindow::handleChar(SkUnichar uni) {
    if (this->onHandleChar(uni))
        return true;

    SkView* focus = this->getFocusView();
    if (focus == nullptr)
        focus = this;

    SkEvent evt(SK_EventType_Unichar);
    evt.setFast32(uni);
    return focus->doEvent(evt);
}

bool SkWindow::handleKey(SkKey key) {
    if (key == kNONE_SkKey)
        return false;

    if (this->onHandleKey(key))
        return true;

    // send an event to the focus-view
    {
        SkView* focus = this->getFocusView();
        if (focus == nullptr)
            focus = this;

        SkEvent evt(SK_EventType_Key);
        evt.setFast32(key);
        if (focus->doEvent(evt))
            return true;
    }

    if (key == kUp_SkKey || key == kDown_SkKey) {
        if (this->moveFocus(key == kUp_SkKey ? kPrev_FocusDirection : kNext_FocusDirection) == nullptr)
            this->onSetFocusView(nullptr);
        return true;
    }
    return false;
}

bool SkWindow::handleKeyUp(SkKey key) {
    if (key == kNONE_SkKey)
        return false;

    if (this->onHandleKeyUp(key))
        return true;

    //send an event to the focus-view
    {
        SkView* focus = this->getFocusView();
        if (focus == nullptr)
            focus = this;

        //should this one be the same?
        SkEvent evt(SK_EventType_KeyUp);
        evt.setFast32(key);
        if (focus->doEvent(evt))
            return true;
    }
    return false;
}

void SkWindow::addMenu(SkOSMenu* menu) {
    *fMenus.append() = menu;
    this->onAddMenu(menu);
}

void SkWindow::setTitle(const char title[]) {
    if (nullptr == title) {
        title = "";
    }
    fTitle.set(title);
    this->onSetTitle(title);
}

bool SkWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(SK_EventDelayInval)) {
        for (SkRegion::Iterator iter(fDirtyRgn); !iter.done(); iter.next())
            this->onHandleInval(iter.rect());
        fWaitingOnInval = false;
        return true;
    }
    return this->INHERITED::onEvent(evt);
}

bool SkWindow::onGetFocusView(SkView** focus) const {
    if (focus)
        *focus = fFocusView;
    return true;
}

bool SkWindow::onSetFocusView(SkView* focus) {
    if (fFocusView != focus) {
        if (fFocusView)
            fFocusView->onFocusChange(false);
        fFocusView = focus;
        if (focus)
            focus->onFocusChange(true);
    }
    return true;
}

void SkWindow::onHandleInval(const SkIRect&) {
}

bool SkWindow::onHandleChar(SkUnichar) {
    return false;
}

bool SkWindow::onHandleKey(SkKey) {
    return false;
}

bool SkWindow::onHandleKeyUp(SkKey) {
    return false;
}

bool SkWindow::handleClick(int x, int y, Click::State state, void *owner,
                           unsigned modifierKeys) {
    return this->onDispatchClick(x, y, state, owner, modifierKeys);
}

bool SkWindow::onDispatchClick(int x, int y, Click::State state,
                               void* owner, unsigned modifierKeys) {
    bool handled = false;

    // First, attempt to find an existing click with this owner.
    int index = -1;
    for (int i = 0; i < fClicks.count(); i++) {
        if (owner == fClicks[i]->fOwner) {
            index = i;
            break;
        }
    }

    switch (state) {
        case Click::kDown_State: {
            if (index != -1) {
                delete fClicks[index];
                fClicks.remove(index);
            }
            Click* click = this->findClickHandler(SkIntToScalar(x),
                                                  SkIntToScalar(y), modifierKeys);

            if (click) {
                click->fOwner = owner;
                *fClicks.append() = click;
                SkView::DoClickDown(click, x, y, modifierKeys);
                handled = true;
            }
            break;
        }
        case Click::kMoved_State:
            if (index != -1) {
                SkView::DoClickMoved(fClicks[index], x, y, modifierKeys);
                handled = true;
            }
            break;
        case Click::kUp_State:
            if (index != -1) {
                SkView::DoClickUp(fClicks[index], x, y, modifierKeys);
                delete fClicks[index];
                fClicks.remove(index);
                handled = true;
            }
            break;
        default:
            // Do nothing
            break;
    }
    return handled;
}

#if SK_SUPPORT_GPU

#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "include/gpu/SkGr.h"

GrRenderTarget* SkWindow::renderTarget(const AttachmentInfo& attachmentInfo,
        const GrGLInterface* interface, GrContext* grContext) {
    GrBackendRenderTargetDesc desc;
    desc.fWidth = SkScalarRoundToInt(this->width());
    desc.fHeight = SkScalarRoundToInt(this->height());
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fSampleCnt = attachmentInfo.fSampleCount;
    desc.fStencilBits = attachmentInfo.fStencilBits;
    GrGLint buffer;
    GR_GL_GetIntegerv(interface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    desc.fRenderTargetHandle = buffer;
    return grContext->textureProvider()->wrapBackendRenderTarget(desc);
}

#endif
