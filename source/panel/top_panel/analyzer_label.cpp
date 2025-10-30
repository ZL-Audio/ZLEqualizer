// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "analyzer_label.hpp"

namespace zlpanel {
    AnalyzerLabel::AnalyzerLabel(PluginProcessor&, zlgui::UIBase& base) :
        base_(base),
        control_background_(base),
        label_laf_(base) {
        control_background_.setInterceptsMouseClicks(false, false);
        // control_background_.setBufferedToImage(true);
        addChildComponent(control_background_);

        label_laf_.setFontScale(1.5f);

        analyzer_label_.setText("Analyzer", juce::dontSendNotification);
        for (auto& l : {&analyzer_label_}) {
            l->setInterceptsMouseClicks(false, false);
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            // l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        setAlpha(.5f);
        setInterceptsMouseClicks(true, false);

        base_.getPanelValueTree().addListener(this);
    }

    AnalyzerLabel::~AnalyzerLabel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void AnalyzerLabel::resized() {
        const auto padding = 2 * getPaddingSize(base_.getFontSize());
        const auto bound = getLocalBounds();
        control_background_.setBounds(0, -padding, bound.getWidth(), bound.getHeight() + padding + padding / 4);
        analyzer_label_.setBounds(bound);
    }

    void AnalyzerLabel::mouseDown(const juce::MouseEvent&) {
        const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel));
        base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel, f < .5 ? 1. : 0.);
    }

    void AnalyzerLabel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::kAnalyzerPanel, property)) {
            const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel));
            control_background_.setVisible(f > .5);
            setAlpha(f > .5 ? 1.f : .5f);
        }
    }
}
