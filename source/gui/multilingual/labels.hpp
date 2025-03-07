// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_MULTILINGUAL_LABELS_HPP
#define ZL_INTERFACE_MULTILINGUAL_LABELS_HPP

namespace zlInterface::multilingual {
    enum labels {
        bandBypass,
        bandSolo,
        bandType,
        bandSlope,
        bandStereoMode,
        bandFreq,
        bandGain,
        bandQ,
        bandSelector,
        bandDynamic,
        bandDynamicAuto,
        bandOff,
        bandDynamicBypass,
        bandDynamicSolo,
        bandDynamicRelative,
        bandSideSwap,
        bandDynamicThreshold,
        bandDynamicKnee,
        bandDynamicAttack,
        bandDynamicRelease,
        bandDynamicSideFreq,
        bandDynamicSideQ,
        externalSideChain,
        staticGC,
        bypass,
        scale,
        phaseFlip,
        autoGC,
        loudnessMatch,
        outputGain,
        fftPre,
        fftPost,
        fftSide,
        fftDecay,
        fftSlope,
        lookahead,
        rms,
        smooth,
        highQuality,
        collisionDET,
        collisionStrength,
        collisionScale,
        filterStructure,
        zeroLatency,
        matchTarget,
        matchWeight,
        matchStartLearn,
        matchSave,
        matchSmooth,
        matchSlope,
        matchAlgo,
        matchStartFit,
        matchNumBand,
        dbScale,
        eqMatch,
        minimumPhase,
        stateVariable,
        parallelPhase,
        matchedPhase,
        mixedPhase,
        linearPhase,
        pluginLogo,
        labelNum
    };
}

#endif //ZL_INTERFACE_MULTILINGUAL_LABELS_HPP
