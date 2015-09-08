/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualBench.h"

#include "ProcStats.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkGr.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "Stats.h"
#include "VisualLightweightBenchModule.h"
#include "gl/GrGLInterface.h"

DEFINE_bool2(fullscreen, f, true, "Run fullscreen.");

VisualBench::VisualBench(void* hwnd, int argc, char** argv)
    : INHERITED(hwnd)
    , fModule(new VisualLightweightBenchModule(this)) {
    SkCommandLineFlags::Parse(argc, argv);

    this->setTitle();
    this->setupBackend();
}

VisualBench::~VisualBench() {
    INHERITED::detach();
}

void VisualBench::setTitle() {
    SkString title("VisualBench");
    INHERITED::setTitle(title.c_str());
}

SkSurface* VisualBench::createSurface() {
    if (!fSurface) {
        SkSurfaceProps props(INHERITED::getSurfaceProps());
        fSurface.reset(SkSurface::NewRenderTargetDirect(fRenderTarget, &props));
    }

    // The caller will wrap the SkSurface in an SkAutoTUnref
    return SkRef(fSurface.get());
}

bool VisualBench::setupBackend() {
    this->setColorType(kRGBA_8888_SkColorType);
    this->setVisibleP(true);
    this->setClipToBounds(false);

    if (FLAGS_fullscreen) {
        if (!this->makeFullscreen()) {
            SkDebugf("Could not go fullscreen!");
        }
    }
    if (!this->attach(kNativeGL_BackEndType, FLAGS_msaa, &fAttachmentInfo)) {
        SkDebugf("Not possible to create backend.\n");
        INHERITED::detach();
        return false;
    }

    this->setVsync(false);
    this->resetContext();
    return true;
}

void VisualBench::resetContext() {
    fSurface.reset(nullptr);

    fInterface.reset(GrGLCreateNativeInterface());
    SkASSERT(fInterface);

    // setup contexts
    fContext.reset(GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)fInterface.get()));
    SkASSERT(fContext);

    // setup rendertargets
    this->setupRenderTarget();
}

void VisualBench::setupRenderTarget() {
    if (fContext) {
        fRenderTarget.reset(this->renderTarget(fAttachmentInfo, fInterface, fContext));
    }
}

void VisualBench::draw(SkCanvas* canvas) {
    fModule->draw(canvas);

    // Invalidate the window to force a redraw. Poor man's animation mechanism.
    this->inval(nullptr);
}

void VisualBench::onSizeChange() {
    this->setupRenderTarget();
}

bool VisualBench::onHandleChar(SkUnichar unichar) {
    return true;
}

// Externally declared entry points
void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
}

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    return new VisualBench(hwnd, argc, argv);
}

