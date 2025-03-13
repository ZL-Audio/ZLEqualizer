// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"
#include "../../../state/state_definitions.hpp"
#include "side_panel.hpp"
#include "reset_attach.hpp"

namespace zlPanel {
    class SinglePanel final : public juce::Component,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
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

        void updateVisible() {
            if (active.load() != isVisible()) {
                setVisible(active.load());
                if (!isVisible()) {
                    avoidRepaint.store(true);
                }
            }
        }

        bool checkRepaint();

        void setScale(const double x) {
            scale.store(x);
            baseF.setGain(static_cast<double>(zlDSP::gain::range.snapToLegalValue(
                static_cast<float>(currentBaseGain.load() * x))));
            targetF.setGain(static_cast<double>(zlDSP::targetGain::range.snapToLegalValue(
                static_cast<float>(currentTargetGain.load() * x))));
        }

        void run(float physicalPixelScaleFactor);

        void lookAndFeelChanged() override;

    private:
        juce::Path curvePath, strokePath, shadowPath, dynPath;
        juce::Path recentCurvePath, recentShadowPath, recentDynPath;
        juce::SpinLock curveLock, shadowLock, dynLock;

        size_t idx;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlDSP::Controller<double> &controllerRef;
        zlPanel::ResetAttach resetAttach;
        zlFilter::Ideal<double, 16> &baseF, &targetF, &mainF;

        std::atomic<bool> dynON, selected, active;
        std::atomic<float> maximumDB;
        AtomicBound atomicBound;
        AtomicPoint atomicBottomLeft, atomicBottomRight;
        std::atomic<float> curveThickness{0.f};

        std::atomic<bool> toRepaint{false};
        std::atomic<bool> avoidRepaint{true};
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

        void handleAsyncUpdate() override;
    };
} // zlPanel
