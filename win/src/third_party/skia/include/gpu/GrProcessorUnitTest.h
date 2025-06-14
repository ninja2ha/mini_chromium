/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorUnitTest_DEFINED
#define GrProcessorUnitTest_DEFINED

#include "include/gpu/GrTestUtils.h"
#include "include/core/SkTArray.h"
#include "include/core/SkTypes.h"

class SkMatrix;
class GrCaps;
class GrContext;
class GrRenderTarget;
struct GrProcessorTestData;

namespace GrProcessorUnitTest {

// Used to access the dummy textures in TestCreate procs.
enum {
    kSkiaPMTextureIdx = 0,
    kAlphaTextureIdx = 1,
};

/** This allows parent FPs to implement a test create with known leaf children in order to avoid
creating an unbounded FP tree which may overflow various shader limits. */
const GrFragmentProcessor* CreateChildFP(GrProcessorTestData*);

}

/*
 * GrProcessorTestData is an argument struct to TestCreate functions
 * fTextures are valid textures that can optionally be used to construct
 * GrTextureAccesses. The first texture has config kSkia8888_GrPixelConfig and the second has
 * kAlpha_8_GrPixelConfig. TestCreate functions are also free to create additional textures using
 * the GrContext.
 */
struct GrProcessorTestData {
    GrProcessorTestData(SkRandom* random,
                        GrContext* context,
                        const GrCaps* caps,
                        const GrRenderTarget* rt,
                        GrTexture* textures[2])
        : fRandom(random)
        , fContext(context)
        , fCaps(caps)
        , fRenderTarget(rt) {
        fTextures[0] = textures[0];
        fTextures[1] = textures[1];
    }
    SkRandom* fRandom;
    GrContext* fContext;
    const GrCaps* fCaps;
    const GrRenderTarget* fRenderTarget;
    GrTexture* fTextures[2];
};

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

class GrProcessor;
class GrTexture;

template <class Processor> class GrProcessorTestFactory : SkNoncopyable {
public:
    typedef const Processor* (*CreateProc)(GrProcessorTestData*);

    GrProcessorTestFactory(CreateProc createProc) {
        fCreateProc = createProc;
        GetFactories()->push_back(this);
    }

    /** Pick a random factory function and create a processor.  */
    static const Processor* Create(GrProcessorTestData* data) {
        VerifyFactoryCount();
        SkASSERT(GetFactories()->count());
        uint32_t idx = data->fRandom->nextRangeU(0, GetFactories()->count() - 1);
        return CreateIdx(idx, data);
    }

    /** Number of registered factory functions */
    static int Count() { return GetFactories()->count(); }

    /** Use factory function at Index idx to create a processor. */
    static const Processor* CreateIdx(int idx, GrProcessorTestData* data) {
        GrProcessorTestFactory<Processor>* factory = (*GetFactories())[idx];
        return factory->fCreateProc(data);
    }

    /*
     * A test function which verifies the count of factories.
     */
    static void VerifyFactoryCount();

private:
    CreateProc fCreateProc;

    static SkTArray<GrProcessorTestFactory<Processor>*, true>* GetFactories();
};

/** GrProcessor subclasses should insert this macro in their declaration to be included in the
 *  program generation unit test.
 */
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrGeometryProcessor> gTestFactory SK_UNUSED;                     \
    static const GrGeometryProcessor* TestCreate(GrProcessorTestData*)

#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrFragmentProcessor> gTestFactory SK_UNUSED;                     \
    static const GrFragmentProcessor* TestCreate(GrProcessorTestData*)

#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    static GrProcessorTestFactory<GrXPFactory> gTestFactory SK_UNUSED;                             \
    static const GrXPFactory* TestCreate(GrProcessorTestData*)

/** GrProcessor subclasses should insert this macro in their implementation file. They must then
 *  also implement this static function:
 *      GrProcessor* TestCreate(GrProcessorTestData*);
 */
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrFragmentProcessor> Effect :: gTestFactory(Effect :: TestCreate)

#define GR_DEFINE_XP_FACTORY_TEST(Factory)                                                         \
    GrProcessorTestFactory<GrXPFactory> Factory :: gTestFactory(Factory :: TestCreate)

#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrGeometryProcessor> Effect :: gTestFactory(Effect :: TestCreate)

#else // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static const GrFragmentProcessor* TestCreate(GrProcessorTestData*)
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    static const GrXPFactory* TestCreate(GrProcessorTestData*)
#define GR_DEFINE_XP_FACTORY_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static const GrGeometryProcessor* TestCreate(GrProcessorTestData*)
#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(X)

#endif // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#endif
