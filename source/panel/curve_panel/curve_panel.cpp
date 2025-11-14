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
                           multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        background_panel_(p, base, tooltip_helper),
        fft_panel_(p, base, tooltip_helper),
        response_panel_(p, base, tooltip_helper),
        match_fft_panel_(p, base, tooltip_helper),
        scale_panel_(p, base, tooltip_helper),
        output_panel_(p, base, tooltip_helper),
        analyzer_panel_(p, base, tooltip_helper) {
        background_panel_.setBufferedToImage(true);
        addAndMakeVisible(background_panel_);
        addAndMakeVisible(fft_panel_);
        addChildComponent(match_fft_panel_);
        addAndMakeVisible(response_panel_);
        response_panel_.addMouseListener(this, true);
        scale_panel_.setBufferedToImage(false);
        addChildComponent(scale_panel_);
        addChildComponent(output_panel_);
        addChildComponent(analyzer_panel_);
        setInterceptsMouseClicks(false, true);
        base_.getPanelValueTree().addListener(this);
    }

    CurvePanel::~CurvePanel() {
        base_.getPanelValueTree().removeListener(this);
        stopThreads();
    }

    void CurvePanel::paintOverChildren(juce::Graphics&) {
        if (fft_panel_.isVisible()) {
            fft_panel_.notify();
        }
        response_panel_.notify();
        if (match_fft_panel_.isVisible()) {
            match_fft_panel_.notify();
        }
    }

    void CurvePanel::resized() {
        const auto bound = getLocalBounds();
        background_panel_.setBounds(bound);
        fft_panel_.setBounds(bound);
        response_panel_.setBounds(bound);
        match_fft_panel_.setBounds(bound);

        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto output_width = output_panel_.getIdealWidth();
        const auto output_height = output_panel_.getIdealHeight();
        output_panel_.setBounds(bound.getWidth() - output_width - 2 * padding, 0, output_width, output_height);

        const auto analyzer_width = analyzer_panel_.getIdealWidth();
        const auto analyzer_height = analyzer_panel_.getIdealHeight();
        analyzer_panel_.setBounds(0, 0, analyzer_width, analyzer_height);

        scale_panel_.setBounds(bound.withLeft(bound.getWidth() - scale_panel_.getIdealWidth()));
    }

    void CurvePanel::mouseDown(const juce::MouseEvent&) {
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, 0.f);
        base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel, 0.f);
    }

    void CurvePanel::repaintCallBack() {
        repaint();
        response_panel_.repaintCallBack();
    }

    void CurvePanel::repaintCallBackSlow() {
        response_panel_.repaintCallBackSlow();
        output_panel_.repaintCallBackSlow();
        analyzer_panel_.repaintCallBackSlow();
        scale_panel_.repaintCallBackSlow();
    }

    void CurvePanel::updateBand() {
        response_panel_.updateBand();
    }

    void CurvePanel::updateSampleRate(const double sample_rate) {
        background_panel_.updateSampleRate(sample_rate);
        fft_panel_.updateSampleRate(sample_rate);
        response_panel_.updateSampleRate(sample_rate);
        match_fft_panel_.updateSampleRate(sample_rate);
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

    void CurvePanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kMatchPanel, property)) {
            const auto f = static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kMatchPanel));
            const auto idx = static_cast<int>(std::round(f));
            match_fft_panel_.setVisible(idx > 0);
            scale_panel_.setVisible(idx > 0);
            fft_panel_.setVisible(idx == 0);
            response_panel_.setVisible(idx != 1 && idx != 2);
            response_panel_.turnMatchON(idx > 0);
        }
    }
}
