/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPanose_DEFINED
#define SkPanose_DEFINED

#include "src/sfnt/SkOTTableTypes.h"
#include "src/sfnt/SkTypedEnum.h"

#pragma pack(push, 1)

struct SkPanose {
    //This value changes the meaning of the following 9 bytes.
    struct FamilyType {
        SK_TYPED_ENUM(Value, SK_OT_BYTE,
            ((Any, 0))
            ((NoFit, 1))
            ((TextAndDisplay, 2))
            ((Script, 3))
            ((Decorative, 4))
            ((Pictoral, 5))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } bFamilyType;

    union Data {
        struct TextAndDisplay {
            struct SerifStyle {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Cove, 2))
                    ((ObtuseCove, 3))
                    ((SquareCove, 4))
                    ((ObtuseSquareCove, 5))
                    ((Square, 6))
                    ((Thin, 7))
                    ((Bone, 8))
                    ((Exaggerated, 9))
                    ((Triangle, 10))
                    ((NormalSans, 11))
                    ((ObtuseSans, 12))
                    ((PerpSans, 13))
                    ((Flared, 14))
                    ((Rounded, 15))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bSerifStyle;

            struct Weight {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((VeryLight, 2))
                    ((Light, 3))
                    ((Thin, 4))
                    ((Book, 5))
                    ((Medium, 6))
                    ((Demi, 7))
                    ((Bold, 8))
                    ((Heavy, 9))
                    ((Black, 10))
                    ((ExtraBlack, 11))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bWeight;

            struct Proportion {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((OldStyle, 2))
                    ((Modern, 3))
                    ((EvenWidth, 4))
                    ((Expanded, 5))
                    ((Condensed, 6))
                    ((VeryExpanded, 7))
                    ((VeryCondensed, 8))
                    ((Monospaced, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bProportion;

            struct Contrast {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((None, 2))
                    ((VeryLow, 3))
                    ((Low, 4))
                    ((MediumLow, 5))
                    ((Medium, 6))
                    ((MediumHigh, 7))
                    ((High, 8))
                    ((VeryHigh, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bContrast;

#ifdef SK_WIN_PANOSE
            //This is what Windows (and FontForge and Apple TT spec) define.
            //The Impact font uses 9.
            struct StrokeVariation {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((GradualDiagonal, 2))
                    ((GradualTransitional, 3))
                    ((GradualVertical, 4))
                    ((GradualHorizontal, 5))
                    ((RapidVertical, 6))
                    ((RapidHorizontal, 7))
                    ((InstantVertical, 8))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bStrokeVariation;
#else
            //Stroke variation description in OT OS/2 ver0,ver1 is incorrect.
            //This is what HP Panose says.
            struct StrokeVariation {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((NoVariation, 2))
                    ((Gradual_Diagonal, 3))
                    ((Gradual_Transitional, 4))
                    ((Gradual_Vertical, 5))
                    ((Gradual_Horizontal, 6))
                    ((Rapid_Vertical, 7))
                    ((Rapid_Horizontal, 8))
                    ((Instant_Vertical, 9))
                    ((Instant_Horizontal, 10))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bStrokeVariation;
#endif

            struct ArmStyle {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((StraightArms_Horizontal, 2))
                    ((StraightArms_Wedge, 3))
                    ((StraightArms_Vertical, 4))
                    ((StraightArms_SingleSerif, 5))
                    ((StraightArms_DoubleSerif, 6))
                    ((NonStraightArms_Horizontal, 7))
                    ((NonStraightArms_Wedge, 8))
                    ((NonStraightArms_Vertical, 9))
                    ((NonStraightArms_SingleSerif, 10))
                    ((NonStraightArms_DoubleSerif, 11))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bArmStyle;

            struct Letterform {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Normal_Contact, 2))
                    ((Normal_Weighted, 3))
                    ((Normal_Boxed, 4))
                    ((Normal_Flattened, 5))
                    ((Normal_Rounded, 6))
                    ((Normal_OffCenter, 7))
                    ((Normal_Square, 8))
                    ((Oblique_Contact, 9))
                    ((Oblique_Weighted, 10))
                    ((Oblique_Boxed, 11))
                    ((Oblique_Flattened, 12))
                    ((Oblique_Rounded, 13))
                    ((Oblique_OffCenter, 14))
                    ((Oblique_Square, 15))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bLetterform;

            struct Midline {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Standard_Trimmed, 2))
                    ((Standard_Pointed, 3))
                    ((Standard_Serifed, 4))
                    ((High_Trimmed, 5))
                    ((High_Pointed, 6))
                    ((High_Serifed, 7))
                    ((Constant_Trimmed, 8))
                    ((Constant_Pointed, 9))
                    ((Constant_Serifed, 10))
                    ((Low_Trimmed, 11))
                    ((Low_Pointed, 12))
                    ((Low_Serifed, 13))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bMidline;

            struct XHeight {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Constant_Small, 2))
                    ((Constant_Standard, 3))
                    ((Constant_Large, 4))
                    ((Ducking_Small, 5))
                    ((Ducking_Standard, 6))
                    ((Ducking_Large, 7))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bXHeight;
        } textAndDisplay;

        struct Script {
            struct ToolKind {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((FlatNib, 2))
                    ((PressurePoint, 3))
                    ((Engraved, 4))
                    ((Ball, 5))
                    ((Brush, 6))
                    ((Rough, 7))
                    ((FeltPen, 8))
                    ((WildBrush, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bToolKind;

            struct Weight {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((VeryLight, 2))
                    ((Light, 3))
                    ((Thin, 4))
                    ((Book, 5))
                    ((Medium, 6))
                    ((Demi, 7))
                    ((Bold, 8))
                    ((Heavy, 9))
                    ((Black, 10))
                    ((ExtraBlack, 11))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bWeight;

            struct Spacing {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((ProportionalSpaced, 2))
                    ((Monospaced, 3))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bSpacing;

            struct AspectRatio {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((VeryCondensed, 2))
                    ((Condensed, 3))
                    ((Normal, 4))
                    ((Expanded, 5))
                    ((VeryExpanded, 6))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspectRatio;

            struct Contrast {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((None, 2))
                    ((VeryLow, 3))
                    ((Low, 4))
                    ((MediumLow, 5))
                    ((Medium, 6))
                    ((MediumHigh, 7))
                    ((High, 8))
                    ((VeryHigh, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bContrast;

            struct Topology {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Roman_Disconnected, 2))
                    ((Roman_Trailing, 3))
                    ((Roman_Connected, 4))
                    ((Cursive_Disconnected, 5))
                    ((Cursive_Trailing, 6))
                    ((Cursive_Connected, 7))
                    ((Blackletter_Disconnected, 8))
                    ((Blackletter_Trailing, 9))
                    ((Blackletter_Connected, 10))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bTopology;

            struct Form {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Upright_NoWrapping, 2))
                    ((Upright_SomeWrapping, 3))
                    ((Upright_MoreWrapping, 4))
                    ((Upright_ExtremeWrapping, 5))
                    ((Oblique_NoWrapping, 6))
                    ((Oblique_SomeWrapping, 7))
                    ((Oblique_MoreWrapping, 8))
                    ((Oblique_ExtremeWrapping, 9))
                    ((Exaggerated_NoWrapping, 10))
                    ((Exaggerated_SomeWrapping, 11))
                    ((Exaggerated_MoreWrapping, 12))
                    ((Exaggerated_ExtremeWrapping, 13))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bForm;

            struct Finials {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((None_NoLoops, 2))
                    ((None_ClosedLoops, 3))
                    ((None_OpenLoops, 4))
                    ((Sharp_NoLoops, 5))
                    ((Sharp_ClosedLoops, 6))
                    ((Sharp_OpenLoops, 7))
                    ((Tapered_NoLoops, 8))
                    ((Tapered_ClosedLoops, 9))
                    ((Tapered_OpenLoops, 10))
                    ((Round_NoLoops, 11))
                    ((Round_ClosedLoops, 12))
                    ((Round_OpenLoops, 13))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bFinials;

            struct XAscent {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((VeryLow, 2))
                    ((Low, 3))
                    ((Medium, 4))
                    ((High, 5))
                    ((VeryHigh, 6))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bXAscent;
        } script;

        struct Decorative {
            struct Class {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Derivative, 2))
                    ((NonStandard_Topology, 3))
                    ((NonStandard_Elements, 4))
                    ((NonStandard_Aspect, 5))
                    ((Initials, 6))
                    ((Cartoon, 7))
                    ((PictureStems, 8))
                    ((Ornamented, 9))
                    ((TextAndBackground, 10))
                    ((Collage, 11))
                    ((Montage, 12))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bClass;

            struct Weight {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((VeryLight, 2))
                    ((Light, 3))
                    ((Thin, 4))
                    ((Book, 5))
                    ((Medium, 6))
                    ((Demi, 7))
                    ((Bold, 8))
                    ((Heavy, 9))
                    ((Black, 10))
                    ((ExtraBlack, 11))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bWeight;

            struct Aspect {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((SuperCondensed, 2))
                    ((VeryCondensed, 3))
                    ((Condensed, 4))
                    ((Normal, 5))
                    ((Extended, 6))
                    ((VeryExtended, 7))
                    ((SuperExtended, 8))
                    ((Monospaced, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspect;

            struct Contrast {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((None, 2))
                    ((VeryLow, 3))
                    ((Low, 4))
                    ((MediumLow, 5))
                    ((Medium, 6))
                    ((MediumHigh, 7))
                    ((High, 8))
                    ((VeryHigh, 9))
                    ((HorizontalLow, 10))
                    ((HorizontalMedium, 11))
                    ((HorizontalHigh, 12))
                    ((Broken, 13))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bContrast;

            struct SerifVariant {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Cove, 2))
                    ((ObtuseCove, 3))
                    ((SquareCove, 4))
                    ((ObtuseSquareCove, 5))
                    ((Square, 6))
                    ((Thin, 7))
                    ((Oval, 8))
                    ((Exaggerated, 9))
                    ((Triangle, 10))
                    ((NormalSans, 11))
                    ((ObtuseSans, 12))
                    ((PerpendicularSans, 13))
                    ((Flared, 14))
                    ((Rounded, 15))
                    ((Script, 16))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bSerifVariant;

            struct Treatment {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((None_StandardSolidFill, 2))
                    ((White_NoFill, 3))
                    ((PatternedFill, 4))
                    ((ComplexFill, 5))
                    ((ShapedFill, 6))
                    ((DrawnDistressed, 7))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bTreatment;

            struct Lining {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((None, 2))
                    ((Inline, 3))
                    ((Outline, 4))
                    ((Engraved, 5))
                    ((Shadow, 6))
                    ((Relief, 7))
                    ((Backdrop, 8))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bLining;

            struct Topology {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Standard, 2))
                    ((Square, 3))
                    ((MultipleSegment, 4))
                    ((DecoWacoMidlines, 5))
                    ((UnevenWeighting, 6))
                    ((DiverseArms, 7))
                    ((DiverseForms, 8))
                    ((LombardicForms, 9))
                    ((UpperCaseInLowerCase, 10))
                    ((ImpliedTopology, 11))
                    ((HorseshoeEandA, 12))
                    ((Cursive, 13))
                    ((Blackletter, 14))
                    ((SwashVariance, 15))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bTopology;

            struct RangeOfCharacters {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((ExtendedCollection, 2))
                    ((Litterals, 3))
                    ((NoLowerCase, 4))
                    ((SmallCaps, 5))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bRangeOfCharacters;
        } decorative;

        struct Pictoral {
            struct Kind {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((Montages, 2))
                    ((Pictures, 3))
                    ((Shapes, 4))
                    ((Scientific, 5))
                    ((Music, 6))
                    ((Expert, 7))
                    ((Patterns, 8))
                    ((Boarders, 9))
                    ((Icons, 10))
                    ((Logos, 11))
                    ((IndustrySpecific, 12))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bKind;

            struct Weight {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((NoFit, 1))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bWeight;

            struct Spacing {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((ProportionalSpaced, 2))
                    ((Monospaced, 3))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bSpacing;

            struct AspectRatioAndContrast {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((NoFit, 1))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspectRatioAndContrast;

            struct AspectRatio94 {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((NoWidth, 2))
                    ((ExceptionallyWide, 3))
                    ((SuperWide, 4))
                    ((VeryWide, 5))
                    ((Wide, 6))
                    ((Normal, 7))
                    ((Narrow, 8))
                    ((VeryNarrow, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspectRatio94;

            struct AspectRatio119 {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((NoWidth, 2))
                    ((ExceptionallyWide, 3))
                    ((SuperWide, 4))
                    ((VeryWide, 5))
                    ((Wide, 6))
                    ((Normal, 7))
                    ((Narrow, 8))
                    ((VeryNarrow, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspectRatio119;

             struct AspectRatio157 {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((NoWidth, 2))
                    ((ExceptionallyWide, 3))
                    ((SuperWide, 4))
                    ((VeryWide, 5))
                    ((Wide, 6))
                    ((Normal, 7))
                    ((Narrow, 8))
                    ((VeryNarrow, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspectRatio157;

            struct AspectRatio163 {
                SK_TYPED_ENUM(Value, SK_OT_BYTE,
                    ((Any, 0))
                    ((NoFit, 1))
                    ((NoWidth, 2))
                    ((ExceptionallyWide, 3))
                    ((SuperWide, 4))
                    ((VeryWide, 5))
                    ((Wide, 6))
                    ((Normal, 7))
                    ((Narrow, 8))
                    ((VeryNarrow, 9))
                    SK_SEQ_END,
                (value)SK_SEQ_END)
            } bAspectRatio163;
        } pictoral;
    } data;
};

#pragma pack(pop)


static_assert(sizeof(SkPanose) == 10, "sizeof_SkPanose_not_10");

#endif
