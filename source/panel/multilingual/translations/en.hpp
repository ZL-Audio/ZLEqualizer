// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

#pragma once

#include <array>

namespace zlpanel::multilingual::en {
    inline constexpr std::array kTexts = {
        // band basic controls
        "Toggles band bypass.",
        "Toggles band solo audition.",
        "Selects the filter type.",
        "Selects the filter slope. A higher slope produces a steeper frequency response curve.",
        "Selects the stereo channel mode.",
        "Adjusts the center or cutoff frequency.",
        "Adjusts the base gain and dynamic target gain.",
        "Adjusts the Q factor. A higher Q narrows the bandwidth.",
        "Toggles dynamic EQ behavior for this band.",
        "Turns off this band.",
        "Selects the previous band.",
        "Selects the next band.",
        "Toggles pass scaling for Low Pass and High Pass filters.",
        "Adjusts the magnitude scale of the pass filter.",
        // band dynamic controls
        "Toggles dynamic processing bypass for this band.",
        "Toggles dynamic threshold learning. Disabling it sets Threshold and Knee automatically.",
        "Toggles relative threshold mode for dynamic processing.",
        "Toggles swapping of the side-chain stereo channels.",
        "Adjusts the dynamic processing threshold.",
        "Adjusts the dynamic processing knee width.",
        "Adjusts the dynamic processing attack time.",
        "Adjusts the dynamic processing release time.",
        "Links the side-chain filter frequency and Q to this band.",
        "Selects the side-chain filter type.",
        "Selects the side-chain filter slope.",
        "Adjusts the side-chain filter frequency.",
        "Adjusts the side-chain filter Q factor.",
        "Toggles extra dynamic detection settings.",
        "Adjusts the RMS integration window length for level detection.",
        "Adjusts the mix between peak and RMS level detection.",
        "Adjusts the smoothing amount applied to the dynamic envelope.",
        // top controls
        "Toggles the UI settings panel.",
        "Toggles the preset browser.",
        "Toggles the analyzer settings panel.",
        "Toggles the output settings panel.",
        "Selects the filter structure.",
        "Standard classic digital structure. Not suitable for aggressive automation.",
        "Stable structure used in synth filters and crossovers. Suitable for aggressive automation, but introduces a large phase shift.",
        "Processes gentle Shelf and Peak filters in parallel for efficient and natural dynamic processing.",
        "Emulates analog prototype magnitude and phase response. Do not automate filter parameters in this mode.",
        "Emulates analog prototype magnitude response and cleans up high-end phase. Do not automate filter parameters in this mode.",
        "Emulates analog prototype magnitude response and cleans up mid- and high-end phase. Do not automate filter parameters in this mode.",
        "Toggles plugin bypass.",
        "Toggles the external side-chain input.",
        "Toggles the EQ match panel.",
        // preset browser
        "Opens the presets folder in the system file manager.",
        "Deletes the selected preset group.",
        "Deletes the selected preset.",
        "Closes the preset browser.",
        // output controls
        "Adjusts the additional output gain.",
        "Adjusts the overall scale of all filters' base and target gain.",
        "Toggles Static Gain Compensation (SGC). SGC is approximate, but does not affect dynamics.",
        "Toggles loudness gain matching. Enabling measures input and output loudness; disabling sets Output Gain to their difference.",
        "Toggles Auto Gain Compensation (AGC). AGC is more accurate over time, but affects dynamics.",
        "Toggles phase inversion of the output signal.",
        "Adjusts the lookahead time for the side-chain signal.",
        // analyzer controls
        "Toggles display of the input signal spectrum.",
        "Toggles display of the output signal spectrum.",
        "Toggles display of the side-chain signal spectrum.",
        "Selects the decay speed of the spectrum analyzer.",
        "Selects the tilt slope of the spectrum analyzer.",
        "Selects the frequency smoothing width of the spectrum analyzer.",
        "Selects the frequency smoothing mode.\nOCT: constant fractional-octave smoothing.\nERB: equivalent rectangular bandwidth smoothing.",
        "Toggles spectrum freezing when hovering the pointer over the analyzer.",
        "Selects the stereo channel mode displayed by the spectrum analyzer.",
        "Toggles frequency collision detection.",
        "Adjusts the collision detection strength.",
        "Toggles visibility of the numeric value readout.",
        "Toggles visibility of the level meter panel.",
        // curve scale & piano roll
        "Selects the decibel display range for the EQ response curve.",
        "Selects the upper decibel ceiling for the spectrum analyzer display.",
        "Selects the minimum decibel floor for the spectrum analyzer display.",
        "Toggles visibility of the piano roll.",
        // eq match controls
        "Saves the target curve to a preset file.",
        "Toggles manual drawing on the difference curve.",
        "Selects the target curve source.",
        "Learns the target curve from the side-chain signal.",
        "Sets the target curve to a flat line.",
        "Loads the target curve from a preset file.",
        "Sets the target curve to a balanced curve.",
        "Sets the target curve to a natural curve.",
        "Adjusts the vertical offset of the difference curve.",
        "Adjusts the vertical scale of the difference curve.",
        "Adjusts the tilt slope of the difference curve.",
        "Selects the maximum allowed gain magnitude for EQ matching.",
        "Starts fitting EQ bands to the difference curve.",
        "Adjusts the number of EQ bands used for fitting.",
        "Analyzing main-chain and side-chain signals.\nSelect the target curve source and adjust the difference curve once it stabilizes before starting the fit.",
        "Fitting EQ bands to the difference curve. This may take a few seconds.",
        "Fitting complete. You can adjust the number of fitted bands if needed.",
        " "
    };
}
