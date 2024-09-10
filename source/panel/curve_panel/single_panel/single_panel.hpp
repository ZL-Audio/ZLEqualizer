// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SINGLE_PANEL_HPP
#define ZLEqualizer_SINGLE_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../dsp/farbot/RealtimeObject.hpp"
#include "../static_omega_array.hpp"
#include "../../../state/state_definitions.hpp"
#include "side_panel.hpp"

namespace zlPanel {
    class SinglePanel final : public juce::Component,
                              private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit SinglePanel(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base,
                             zlDSP::Controller<double> &controller);

        ~SinglePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void setMaximumDB(const float x) {
            maximumDB.store(x);
            toRepaint.store(true);
        }

        bool checkRepaint();

        bool willRepaint() const;

        void run();

    private:
        juce::Path curvePath, shadowPath, dynPath;
        farbot::RealtimeObject<juce::Path, farbot::RealtimeObjectOptions::realtimeMutatable>
                recentCurvePath, recentShadowPath, recentDynPath;

        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<bool> dynON, selected, actived;
        zlDSP::Controller<double> &controllerRef;
        zlFilter::DynamicIIR<double> &filter;
        zlFilter::IIR<double> &baseF, &targetF;
        std::atomic<float> maximumDB;
        std::atomic<float> xx{-100.f}, yy{-100.f}, width{.1f}, height{.1f};

        std::atomic<bool> skipRepaint{false};
        std::atomic<bool> toRepaint{false};
        std::atomic<bool> avoidRepaint{false};
        SidePanel sidePanel;
        std::atomic<float> centeredDB{0.f};
        std::atomic<double> baseFreq{1000.0}, baseGain{0.0};

        static constexpr std::array changeIDs{
            zlDSP::bypass::ID, zlDSP::lrType::ID, zlDSP::dynamicON::ID
        };

        juce::Colour colour;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void drawCurve(juce::Path &path,
                       const std::array<double, zlFilter::frequencies.size()> &dBs,
                       juce::Rectangle<float> bound,
                       bool reverse = false,
                       bool startPath = true);

        inline static float indexToX(const size_t i, const juce::Rectangle<float> bound) {
            return static_cast<float>(i) /
                   static_cast<float>(zlFilter::frequencies.size() - 1) * bound.getWidth() + bound.getX();
        }

        inline static float freqToX(const double freq, const juce::Rectangle<float> bound) {
            const auto portion = std::log(freq / zlFilter::frequencies.front()) / std::log(
                                     zlFilter::frequencies.back() / zlFilter::frequencies.front());
            return static_cast<float>(portion) * bound.getWidth() + bound.getX();
        }

        inline static float dbToY(const float db, const float maxDB, const juce::Rectangle<float> bound) {
            return -db / maxDB * bound.getHeight() * 0.5f + bound.getCentreY();
        }
    };
} // zlPanel

#endif //ZLEqualizer_SINGLE_PANEL_HPP
