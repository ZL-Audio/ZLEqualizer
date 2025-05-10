// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS of A PARTICULAR PURPOSE. See the GNU Affero General Public License of more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

namespace zlgui::multilingual::en {
    static constexpr std::array kTexts = {
        "Press: turn on the selected band.\nRelease: bypass the selected band.",
        "Press: solo the audio affected by the selected frequency band.",
        "Choose the type of the selected frequency band.",
        "Choose the slope of the selected frequency band. A higher slope will make the filter’s response curve change more steeply.",
        "Choose the stereo mode of the selected frequency band.",
        "Adjust the center frequency of the selected frequency band.",
        "Adjust the gain of the selected frequency band.",
        "Adjust the Q factor of the selected frequency band. The larger the Q value, the narrower the bandwidth.",
        "Choose the frequency band with the left/right arrow.",
        "Press: turn on the dynamic behaviour of the selected frequency band.",
        "Press: turn on automatic threshold of the selected frequency band.\nRelease: turn off automatic threshold of the selected frequency band and apply the learned threshold.",
        "Click: turn off the selected frequency band.",
        "Release: bypass the dynamic behaviour of the selected band.",
        "Press: solo the side-chain audio of the selected band.",
        "Press: set the dynamic threshold to the relative mode.\nRelease: set the dynamic threshold to the absolute mode.",
        "Press: swap the side-chain to set it in the opposite stereo mode.",
        "Adjust the threshold of the dynamic behaviour of the selected band.",
        "Adjust the knee width of the dynamic behaviour of the selected band. The actual knee width is the displayed value × 60 dB.",
        "Adjust the attack time of the dynamic behaviour of the selected band.",
        "Adjust the release time of the dynamic behaviour of the selected band.",
        "Adjust the center frequency of the bandpass filter applied to the side-chain signal.",
        "Adjust the Q value of the bandpass filter applied to the side-chain signal.",
        "Press: use external side-chain\nRelease: use internal side-chain.",
        "Press: turn on Static Gain Compensation. SGC estimates the amount of compensation from filters’ parameters. SGC is inaccurate. However, it will NOT affect the dynamic of the signal.",
        "Press: bypass the plugin.",
        "Adjust the scale of all gain parameters.",
        "Press: flip the phase of the output signal.",
        "Press: turn on Automatic Gain Compensation. AGC is accurate in the long term. However, AGC will affect the dynamic of the signal.",
        "Press: when you press it, it starts to measure the integrated loudness of the input signal and the output signal.\nRelease: when you release it, it turns AGC off and updates the Output Gain to the difference between two loudness values.",
        "Adjust the output gain of the plugin.",
        "Choose the state of the input spectrum analyzer. FRZ can freeze the analyzer.",
        "Choose the state of the output spectrum analyzer. FRZ can freeze the analyzer.",
        "Choose the state of the side-chain spectrum analyzer. FRZ can freeze the analyzer.",
        "Adjust the decay speed of the spectrum analyzer.",
        "Adjust the slope of the spectrum analyzer (does not affect the actual signal).",
        "Adjust the lookahead time of of the side-chain signal (by introducing latency).",
        "Adjust the length of RMS. As RMS increases, the attack/release will be slower and smoother.",
        "Adjust the smoothness of the release.",
        "Choose the state of High-Quality option. When high-quality is on, dynamic will adjust the filter state per sample, which smooths dynamic effects and reduces artifacts.",
        "Choose the state of Collision Detection.",
        "Adjust the strength of Collision Detection. If you increase strength, the detected areas will become larger and darker.",
        "Adjust the scale of Collision Detection. If you increase the scale, the detected areas will become darker.",
        "Choose the structure of filters.",
        "Choose the state of Zero Latency. When Zero Latency is on, the extra latency is zero. However, the buffer size will slightly affect dynamic effects and parameter automation.",
        "Choose the target curve.\nSide: learned from the side chain signal.\nPreset: loaded from a preset file.\nFlat: set as a flat -4.5 dB/oct line.",
        "Adjust the weight of the curve learning model. The larger the weight, the more the model learns from the loud part of the signal.",
        "Click: start the curve learning.",
        "Click: save the current target curve as a preset file.",
        "Adjust the smoothness of the difference curve.",
        "Adjust the slope of the difference curve.",
        "Choose the fitting algorithm.\nLD: local gradient-based algorithm (fast).\nGN: global gradient-free algorithm (recommended).\nGN+: global gradient-free algorithm (slow, allows filters to have higher slopes).",
        "Click: start the curve fitting.",
        "Adjust the number of bands used for fitting. When the fitting is completed, the curve fitting model will suggest the number of bands. You can change it afterward.",
        "Choose the dB scale of filter response curves (left) and spectrum analyzers (right).",
        "Click: open the Equalizer Match panel. The Equalizer Match has four steps:\n1. Choose the target curve.\n2. Start the learning.\n3. Adjust the difference.\n4. Start the fitting.",
        "Most common filter structure of equalizers. Filter response is very close to analog prototype. Suitable for modulation.",
        "Most common filter structure of crossovers. Filter causes more phase shift. Suitable for fast modulation.",
        "Peak/Shelf filters are placed parallely. Filter response is different from the displayed curve. Suitable for modulation.",
        "Minimum Phase structure with additional corrections. Filter response is almost identical to analog prototype. NOT suitable for modulation.",
        "Minimum Phase structure with additional corrections. Filter has analog prototype magnitude response and less phase shift at high-end. NOT suitable for modulation.",
        "Filter has analog prototype magnitude response and zero phase response. NOT suitable for modulation. Dynamic effect does not work.",
        "Double Click: open UI Settings."
    };
}
