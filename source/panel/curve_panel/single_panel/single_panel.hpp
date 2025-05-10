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

namespace zlpanel {
    class SinglePanel final : public juce::Component,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
    public:
        explicit SinglePanel(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA,
                             zlgui::UIBase &base,
                             zlp::Controller<double> &controller,
                             zldsp::filter::Ideal<double, 16> &baseFilter,
                             zldsp::filter::Ideal<double, 16> &targetFilter,
                             zldsp::filter::Ideal<double, 16> &mainFilter);

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
            baseF.setGain(static_cast<double>(zlp::gain::range.snapToLegalValue(
                static_cast<float>(currentBaseGain.load() * x))));
            targetF.setGain(static_cast<double>(zlp::targetGain::range.snapToLegalValue(
                static_cast<float>(currentTargetGain.load() * x))));
        }

        void run(float physicalPixelScaleFactor);

        juce::Point<float> getButtonPos() const { return buttonPos.load(); }

        juce::Point<float> getTargetButtonPos() const {return targetButtonPos.load();}

        void lookAndFeelChanged() override;

    private:
        juce::Path curvePath, strokePath, shadowPath, dynPath;
        juce::Path recentCurvePath, recentShadowPath, recentDynPath;
        juce::SpinLock curveLock, shadowLock, dynLock;

        size_t idx;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlp::Controller<double> &controller_ref_;
        zlpanel::ResetAttach resetAttach;
        zldsp::filter::Ideal<double, 16> &baseF, &targetF, &mainF;

        std::atomic<bool> dynON, selected, active;
        std::atomic<float> maximumDB;
        AtomicBound<float> atomicBound;
        AtomicPoint<float> atomicBottomLeft, atomicBottomRight;
        std::atomic<float> curveThickness{0.f};

        std::atomic<bool> toRepaint{false};
        std::atomic<bool> avoidRepaint{true};
        AtomicPoint<float> buttonPos, buttonCurvePos, targetButtonPos;
        std::atomic<double> currentBaseGain{0.0}, currentTargetGain{0.0};
        std::atomic<double> scale{1.0};

        static constexpr std::array changeIDs{
            zlp::bypass::ID, zlp::lrType::ID,
            zlp::dynamicON::ID
        };

        static constexpr std::array paraIDs{
            zlp::fType::ID, zlp::slope::ID,
            zlp::freq::ID, zlp::gain::ID, zlp::Q::ID,
            zlp::targetGain::ID, zlp::targetQ::ID
        };

        juce::Colour colour;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
} // zlpanel
