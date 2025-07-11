/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLContext_DEFINED
#define GrGLContext_DEFINED

#include "include/gpu/gl/GrGLExtensions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLUtil.h"

struct GrContextOptions;

/**
 * Encapsulates information about an OpenGL context including the OpenGL
 * version, the GrGLStandard type of the context, and GLSL version.
 */
class GrGLContextInfo : public SkRefCnt {
public:
    GrGLStandard standard() const { return fInterface->fStandard; }
    GrGLVersion version() const { return fGLVersion; }
    GrGLSLGeneration glslGeneration() const { return fGLSLGeneration; }
    GrGLVendor vendor() const { return fVendor; }
    GrGLRenderer renderer() const { return fRenderer; }
    /** What driver is running our GL implementation? This is not necessarily related to the vendor.
        (e.g. Intel GPU being driven by Mesa) */
    GrGLDriver driver() const { return fDriver; }
    GrGLDriverVersion driverVersion() const { return fDriverVersion; }
    const GrGLCaps* caps() const { return fGLCaps.get(); }
    GrGLCaps* caps() { return fGLCaps; }
    bool hasExtension(const char* ext) const {
        return fInterface->hasExtension(ext);
    }

    const GrGLExtensions& extensions() const { return fInterface->fExtensions; }

protected:
    struct ConstructorArgs {
        const GrGLInterface*                fInterface;
        GrGLVersion                         fGLVersion;
        GrGLSLGeneration                    fGLSLGeneration;
        GrGLVendor                          fVendor;
        GrGLRenderer                        fRenderer;
        GrGLDriver                          fDriver;
        GrGLDriverVersion                   fDriverVersion;
        const  GrContextOptions*            fContextOptions;
    };

    GrGLContextInfo(const ConstructorArgs& args);

    SkAutoTUnref<const GrGLInterface>   fInterface;
    GrGLVersion                         fGLVersion;
    GrGLSLGeneration                    fGLSLGeneration;
    GrGLVendor                          fVendor;
    GrGLRenderer                        fRenderer;
    GrGLDriver                          fDriver;
    GrGLDriverVersion                   fDriverVersion;
    SkAutoTUnref<GrGLCaps>              fGLCaps;
};

/**
 * Extension of GrGLContextInfo that also provides access to GrGLInterface.
 */
class GrGLContext : public GrGLContextInfo {
public:
    /**
     * Creates a GrGLContext from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    static GrGLContext* Create(const GrGLInterface* interface, const GrContextOptions& options);

    const GrGLInterface* interface() const { return fInterface; }

private:
    GrGLContext(const ConstructorArgs& args) : INHERITED(args) {}

    typedef GrGLContextInfo INHERITED;
};

#endif
