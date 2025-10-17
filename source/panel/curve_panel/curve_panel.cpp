// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor& p,
                           zlgui::UIBase& base,
                           const multilingual::TooltipHelper& tooltip_helper) :
        background_panel_(p, base, tooltip_helper),
        fft_panel_(p, base, tooltip_helper),
        response_panel_(p, base, tooltip_helper),
        scale_panel_(p, base, tooltip_helper) {
        background_panel_.setBufferedToImage(true);
        addAndMakeVisible(background_panel_);

        addAndMakeVisible(fft_panel_);

        addAndMakeVisible(response_panel_);

        scale_panel_.setBufferedToImage(true);
        addAndMakeVisible(scale_panel_);

        setInterceptsMouseClicks(false, true);
    }

    void CurvePanel::paintOverChildren(juce::Graphics&) {
        if (fft_panel_.isThreadRunning()) {
            fft_panel_.notify();
        }
        if (response_panel_.isThreadRunning()) {
            response_panel_.notify();
        }
    }

    void CurvePanel::resized() {
        auto bound = getLocalBounds();
        background_panel_.setBounds(bound);
        fft_panel_.setBounds(bound);
        response_panel_.setBounds(bound);
        scale_panel_.setBounds(bound.removeFromRight(scale_panel_.getIdealWidth()));
    }

    void CurvePanel::repaintCallBack() {
        fft_panel_.repaint();
    }

    void CurvePanel::repaintCallBackSlow() {
        response_panel_.repaintCallBackSlow();
        scale_panel_.repaintCallBackSlow();
    }

    void CurvePanel::updateBand() {
        response_panel_.updateBand();
    }

    void CurvePanel::updateSampleRate(const double sample_rate) {
        background_panel_.updateSampleRate(sample_rate);
        fft_panel_.updateSampleRate(sample_rate);
        response_panel_.updateSampleRate(sample_rate);
    }

    void CurvePanel::startThreads() {
        fft_panel_.startThread(juce::Thread::Priority::low);
        response_panel_.startThread(juce::Thread::Priority::low);
    }

    void CurvePanel::stopThreads() {
        if (fft_panel_.isThreadRunning()) {
            fft_panel_.stopThread(-1);
        }
        if (response_panel_.isThreadRunning()) {
            response_panel_.stopThread(-1);
        }
    }
}
