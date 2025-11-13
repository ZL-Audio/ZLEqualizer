// Copyright (C) 2025 - zsliu98
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

        kBypass,
        kExternalSideChain,

        kOutputGain,
        kGainScale,
        kStaticGC,
        kLoudnessGC,
        kAutoGC,
        kPhaseFlip,
        kLookahead,

        kFFTPre,
        kFFTPost,
        kFFTSide,
        kFFTSpeed,
        kFFTSlope,
        kFFTFreeze,
        kFFTCollision,
        kFFTCollisionStrength,

        kFilterStructure,
        kMinimumPhase,
        kStateVariable,
        kParallel,
        kMatchedPhase,
        kMixedPhase,
        kZeroPhase,

        kPluginLogo,

        kEQMatch,
        kEQMatchSave,
        kEQMatchTarget,
        kEQMatchTargetSide,
        kEQMatchTargetFlat,
        kEQMatchTargetPreset,
        kEQMatchDiffShift,
        kEQMatchDiffSmooth,
        kEQMatchDiffSlope,
        kEQMatchFit,
        kEQMatchNumBand,
        KEQMatchNotification1,
        KEQMatchNotification2,
        KEQMatchNotification3,
        kEQMatchDiffDraw
    };
}
