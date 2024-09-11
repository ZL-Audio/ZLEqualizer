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
#include "../helpers.hpp"
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
                             zlDSP::Controller<double> &controller,
                             zlFilter::Ideal<double, 16> &baseFilter,
                             zlFilter::Ideal<double, 16> &targetFilter,
                             zlFilter::Ideal<double, 16> &mainFilter);

        ~SinglePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void setMaximumDB(const float x) {
            maximumDB.store(x);
            toRepaint.store(true);
        }

        bool checkRepaint();

        bool willRepaint() const;

        void setScale(const double x) {
            scale.store(x);
            baseF.setGain(static_cast<double>(zlDSP::gain::range.snapToLegalValue(
                static_cast<float>(currentBaseGain.load() * x))));
            targetF.setGain(static_cast<double>(zlDSP::targetGain::range.snapToLegalValue(
                static_cast<float>(currentTargetGain.load() * x))));
        }

        void run();

    private:
        juce::Path curvePath, shadowPath, dynPath;
        juce::Path recentCurvePath, recentShadowPath, recentDynPath;
        juce::SpinLock curveLock, shadowLock, dynLock;

        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;

        zlInterface::UIBase &uiBase;
        std::atomic<bool> dynON, selected, actived;
        zlDSP::Controller<double> &controllerRef;
        zlFilter::Ideal<double, 16> &baseF, &targetF, &mainF;
        std::atomic<float> maximumDB;
        std::atomic<float> xx{-100.f}, yy{-100.f}, width{.1f}, height{.1f};

        std::atomic<bool> skipRepaint{false};
        std::atomic<bool> toRepaint{false};
        std::atomic<bool> avoidRepaint{false};
        SidePanel sidePanel;
        std::atomic<float> centeredDB{0.f};
        std::atomic<double> baseFreq{1000.0}, baseGain{0.0};
        std::atomic<double> currentBaseGain{0.0}, currentTargetGain{0.0};
        std::atomic<double> scale{1.0};

        static constexpr std::array changeIDs{
            zlDSP::bypass::ID, zlDSP::lrType::ID,
            zlDSP::dynamicON::ID
        };

        static constexpr std::array paraIDs{
            zlDSP::fType::ID, zlDSP::slope::ID,
            zlDSP::freq::ID, zlDSP::gain::ID, zlDSP::Q::ID,
            zlDSP::targetGain::ID, zlDSP::targetQ::ID
        };

        juce::Colour colour;

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlPanel

#endif //ZLEqualizer_SINGLE_PANEL_HPP
