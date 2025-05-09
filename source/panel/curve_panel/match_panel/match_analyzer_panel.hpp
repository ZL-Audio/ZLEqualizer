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
#include "match_label.hpp"

namespace zlpanel {
    class MatchAnalyzerPanel final : public juce::Component,
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ValueTree::Listener,
                                     private zlgui::Dragger::Listener {
    public:
        explicit MatchAnalyzerPanel(zldsp::eq_match::EqMatchAnalyzer<double> &analyzer,
                                    juce::AudioProcessorValueTreeState &parametersNA,
                                    zlgui::UIBase &base);

        ~MatchAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void visibilityChanged() override;

        void updatePaths();

        void updateDraggers();

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        zldsp::eq_match::EqMatchAnalyzer<double> &analyzerRef;
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlgui::UIBase &uiBase;
        juce::Path path1{}, path2{}, path3{};
        juce::Path recentPath1{}, recentPath2{}, recentPath3{};
        juce::SpinLock pathLock;
        AtomicPoint<float> leftCorner, rightCorner;
        AtomicBound<float> atomicBound;
        float backgroundAlpha = .5f;
        bool showAverage{true};
        std::atomic<float> dBScale{1.f};
        float currentMaximumDB{zlstate::maximumDB::dBs[static_cast<size_t>(zlstate::maximumDB::defaultI)]};
        std::atomic<float> maximumDB{zlstate::maximumDB::dBs[static_cast<size_t>(zlstate::maximumDB::defaultI)]};
        zlgui::Dragger lowDragger, highDragger, shiftDragger;
        MatchLabel matchLabel;
        static constexpr auto scale = 1.5f;
        size_t preDrawIdx{0};
        float preDrawDB{0.f};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void lookAndFeelChanged() override;

        void dragStarted(zlgui::Dragger *dragger) override {
            juce::ignoreUnused(dragger);
        }

        void dragEnded(zlgui::Dragger *dragger) override {
            juce::ignoreUnused(dragger);
        }

        void draggerValueChanged(zlgui::Dragger *dragger) override;

        void getIdxDBromPoint(const juce::Point<int> &p, size_t &idx, float &dB) const {
            const auto bound = getLocalBounds().toFloat();
            const auto idxInt = juce::roundToInt(
                250.f * (static_cast<float>(p.getX()) - bound.getX()) / bound.getWidth());
            idx = static_cast<size_t>(std::clamp(idxInt, 0, 250));
            const auto yP = (static_cast<float>(p.getY()) - bound.getY()) / bound.getHeight() - .5f;
            dB = -maximumDB.load() * dBScale.load() * yP;
        }
    };
} // zlpanel
