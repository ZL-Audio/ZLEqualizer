// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zlpanel::multilingual {
    enum TooltipLabel {
        // band basic controls
        kBandBypass,
        kBandSolo,
        kBandType,
        kBandSlope,
        kBandStereoMode,
        kBandFreq,
        kBandGain,
        kBandQ,
        kBandDynamic,
        kBandOff,
        kBandPrev,
        kBandNext,
        kBandPassScaleOn,
        kBandPassScale,

        // band dynamic controls
        kBandDynamicBypass,
        kBandDynamicAuto,
        kBandDynamicRelative,
        kBandSideSwap,
        kBandDynamicThreshold,
        kBandDynamicKnee,
        kBandDynamicAttack,
        kBandDynamicRelease,
        kBandDynamicSideLink,
        kBandDynamicSideFilterType,
        kBandDynamicSideFilterSlope,
        kBandDynamicSideFreq,
        kBandDynamicSideQ,
        kBandDynamicExtra,
        kBandDynamicRMSLength,
        kBandDynamicRMSMix,
        kBandDynamicSmooth,

        // top controls
        kPluginLogo,
        kPresetOpen,
        kAnalyzerOpen,
        kOutputOpen,
        kFilterStructure,
        kMinimumPhase,
        kStateVariable,
        kParallel,
        kMatchedPhase,
        kMixedPhase,
        kZeroPhase,
        kBypass,
        kExternalSideChain,
        kEQMatch,

        // preset browser
        kPresetFolder,
        kPresetGroupDelete,
        kPresetDelete,
        kPresetClose,

        // output controls
        kOutputGain,
        kGainScale,
        kStaticGC,
        kLoudnessGC,
        kAutoGC,
        kPhaseFlip,
        kLookahead,

        // analyzer controls
        kFFTPre,
        kFFTPost,
        kFFTSide,
        kFFTSpeed,
        kFFTSlope,
        kFFTSmoothValue,
        kFFTSmoothType,
        kFFTFreeze,
        kFFTStereo,
        kFFTCollision,
        kFFTCollisionStrength,
        kFFTValue,
        kFFTMeter,

        // curve scale & piano roll
        kScaleEQMaxDB,
        kScaleFFTTopDB,
        kScaleFFTMinDB,
        kPianoRoll,

        // eq match controls
        kEQMatchSave,
        kEQMatchDiffDraw,
        kEQMatchTarget,
        kEQMatchTargetSide,
        kEQMatchTargetFlat,
        kEQMatchTargetPreset,
        kEQMatchTargetBalanced,
        kEQMatchTargetNatural,
        kEQMatchDiffShift,
        kEQMatchDiffSmooth,
        kEQMatchDiffSlope,
        kEQMatchLimit,
        kEQMatchFit,
        kEQMatchNumBand,
        kEQMatchNotification1,
        kEQMatchNotification2,
        kEQMatchNotification3,

        kLabelNum
    };
}
