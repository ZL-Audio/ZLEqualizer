// Copyright (C) 2025 - zsliu98
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
    static constexpr std::array kTexts = {
        "Release: bypass the band.",
        "Press: solo the band.",
        "Choose the filter type.",
        "Choose the filter slope. A higher slope will make the filter’s response curve change more steeply.",
        "Choose the stereo mode.",
        "Control the frequency.",
        "Control the base gain and the target gain.",
        "Control the quality factor. The larger the Q value, the narrower the bandwidth.",
        "Press: turn on the dynamic behavior.",
        "Click: turn off the band.",
        "Release: bypass the dynamic behaviour.",
        "Press: turn on the dynamic learning behavior.\nRelease: turn off the dynamic learning behavior and set the Threshold and Knee.\nSee details in the manual.",
        "Press: turn on the dynamic relative behavior.\nSee details in the manual.",
        "Press: change the side-chain stereo mode.",
        "Control the threshold of the dynamic behaviour.\nSee details in the manual.",
        "Control the knee width of the dynamic behaviour.\nSee details in the manual.",
        "Control the attack time of the dynamic behaviour.\nSee details in the manual.",
        "Control the release time of the dynamic behaviour.\nSee details in the manual.",
        "Press: link the band with the side-chain band.",
        "Choose the side-chain filter type.",
        "Choose the side-chain filter slope.",
        "Control the side-chain filter frequency.",
        "Control the side-chain filter quality factor.",

        "Release: bypass the plugin.",
        "Press: use the external side-chain.\nRelease: use the internal side-chain.",

        "Control the additional output gain.",
        "Control the scale of all filters’ base & target gain.",
        "Press: turn on Static Gain Compensation. SGC is inaccurate, but does not affect dynamics.",
        "Press: start to measure the integrated loudness of the input signal and the output signal\nRelease: turn AGC off and update the Output Gain to the difference between the two loudness values.",
        "Press: turn on Auto Gain Compensation. AGC is more accurate in the long term, but affects dynamics.",
        "Press: flip the phase of the output signal.",
        "Control the lookahead of the side-chain signal.",

        "Press: turn on input signal spectrum analyzer",
        "Press: turn on output signal spectrum analyzer",
        "Press: turn on side-chain signal spectrum analyzer",
        "Choose the decay speed of spectrum analyzers.",
        "Choose the tilt slope of spectrum analyzers.",
        "Press: turn on the freezing feature. Hover the mouse over the analyzer to freeze the spectrum.",
        "Press: turn on the collision detection.",
        "Control the collision detection strength.",

        "Choose filter structure.",
        "The standard, classic digital structure. Not suitable for aggressive automation.",
        "The stable structure used in synth filters and crossovers. Suitable for aggressive automation. Causes large phase shift.",
        "Gentle Shelf and Peak filters are processed in parallel. Efficient & Natural dynamic processing.",
        "Mimic analog prototype magnitude & phase response. Do NOT automate filter parameters in this mode.",
        "Mimic analog prototype magnitude response and clean up high-end phase. Do NOT automate filter parameters in this mode.",
        "Mimic analog prototype magnitude response and clean up mid & high-end phase. Do NOT automate filter parameters in this mode.",

        "Double-click: open the UI settings.",

        "Press: open the EQ match panel.",
        "Click: save the target curve to a preset file.",
        "Choose: choose the target curve.",
        "Learn the target curve from the side-chain signal.",
        "Set the target curve as a flat line.",
        "Load the target curve from a preset file.",
        "Control the shift of difference curve.",
        "Control the smoothness of difference curve.",
        "Control the slope of difference curve.",
        "Click: start the fitting process.",
        "Control the number of bands.",
        "The EQ match model is analyzing the main-chain signal and the side-chain signal.\nThe source must be the main-chain signal, but you can choose the target.\nThe difference between the source and the target is the difference.\nOnce the difference becomes stable, you can further adjust it before the fitting process.",
        "The EQ match model is running the fitting process. It should take at most several seconds.",
        "The EQ match model has completed the fitting process. You can now change the number of bands used for fitting.",
        "Press: enable difference curve drawing."
    };
}
