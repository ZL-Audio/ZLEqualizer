// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"
#include "../helper/piano_roll_helper.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor& p,
                           zlgui::UIBase& base,
                           multilingual::TooltipHelper& tooltip_helper) :
        Thread("curve_panel"),
        base_(base),
        value_display_on_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PValueDisplayON::kID)),
        eq_max_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)),
        background_panel_(p, base, tooltip_helper),
        fft_panel_(p, base),
        response_panel_(p, base, tooltip_helper),
        match_fft_panel_(p, base),
        scale_panel_(p, base, tooltip_helper),
        top_panel_(p, base, tooltip_helper),
        output_panel_(p, base, tooltip_helper),
        analyzer_panel_(p, base, tooltip_helper),
        piano_roll_panel_(p, base),
        value_note_panel_(base) {
        background_panel_.setBufferedToImage(true);
        addAndMakeVisible(background_panel_);
        addAndMakeVisible(fft_panel_);
        addChildComponent(match_fft_panel_);
        addAndMakeVisible(response_panel_);
        response_panel_.addMouseListener(this, true);
        scale_panel_.setBufferedToImage(false);
        addChildComponent(scale_panel_);
        top_panel_.setBufferedToImage(true);
        addAndMakeVisible(top_panel_);
        addChildComponent(output_panel_);
        addChildComponent(analyzer_panel_);
        addAndMakeVisible(piano_roll_panel_);
        piano_roll_panel_.addMouseListener(this, true);
        addChildComponent(value_note_panel_);
        updatePianoRollVisibility();
        setInterceptsMouseClicks(false, true);
        base_.getPanelValueTree().addListener(this);
    }

    CurvePanel::~CurvePanel() {
        base_.getPanelValueTree().removeListener(this);
        stopThreads();
    }

    void CurvePanel::paintOverChildren(juce::Graphics&) {
        notify();
        response_panel_.notify();
    }

    void CurvePanel::run() {
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            if (is_match_on_.load(std::memory_order::relaxed)) {
                match_fft_panel_.run(*this);
            } else {
                fft_panel_.run(*this);
            }
        }
    }

    void CurvePanel::resized() {
        const auto bound = getLocalBounds();
        const auto font_size = base_.getFontSize();
        background_panel_.setBounds(bound);

        if (fft_panel_.getBounds() == bound) {
            fft_panel_.resized();
        } else {
            fft_panel_.setBounds(bound);
        }

        const auto plot_bound = bound.withTrimmedTop(top_panel_.getIdealHeight());
        top_panel_.setBounds(bound.withHeight(top_panel_.getIdealHeight()));
        response_panel_.setBounds(plot_bound);
        match_fft_panel_.setBounds(plot_bound);

        const auto padding = getPaddingSize(font_size);
        const auto output_width = output_panel_.getIdealWidth();
        const auto output_height = output_panel_.getIdealHeight();
        output_panel_.setBounds(bound.getWidth() - output_width - 2 * padding, plot_bound.getY(),
                                output_width, output_height);

        const auto analyzer_width = analyzer_panel_.getIdealWidth();
        const auto analyzer_height = analyzer_panel_.getIdealHeight();
        analyzer_panel_.setBounds(getButtonSize(font_size) + 2 * padding, plot_bound.getY(),
                                  analyzer_width, analyzer_height);

        scale_panel_.setBounds(plot_bound.withLeft(bound.getWidth() - scale_panel_.getIdealWidth()));
        top_panel_.setScaleGradientWidth(scale_panel_.getGradientWidth());

        const auto footer_height = piano_roll_helper::getHeight(font_size);
        piano_roll_panel_.setBounds(bound.withTop(bound.getBottom() - footer_height));
        const auto value_width = static_cast<int>(std::round(base_.getFontSize() * 4.5f));
        const auto value_height = footer_height * 2;
        value_note_panel_.setBounds(bound.getRight() - value_width, bound.getBottom() - value_height,
                                    value_width, value_height);
        updateValueNotePanel(true);
    }

    void CurvePanel::mouseDown(const juce::MouseEvent&) {
        base_.setPanelProperty(zlgui::PanelSettingIdx::kOutputPanel, 0.f);
        base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel, 0.f);
    }

    void CurvePanel::repaintCallBack() {
        piano_roll_panel_.repaintCallBack();
        updatePianoRollVisibility();
        repaint();
        response_panel_.repaintCallBack();
    }

    void CurvePanel::repaintCallBackSlow() {
        response_panel_.repaintCallBackSlow();
        output_panel_.repaintCallBackSlow();
        analyzer_panel_.repaintCallBackSlow();
        scale_panel_.repaintCallBackSlow();
        top_panel_.setScaleGradientWidth(scale_panel_.getGradientWidth());
        top_panel_.repaintCallbackSlow();
        updateValueNotePanel();
    }

    void CurvePanel::updateBand() {
        response_panel_.updateBand();
        piano_roll_panel_.updateBand();
        updateValueNotePanel();
    }

    void CurvePanel::updateSampleRate(const double sample_rate) {
        fft_max_ = freq_helper::getFFTMax(sample_rate);
        frequency_max_ = std::max(10.0, std::min(freq_helper::getSliderMax(sample_rate),
                                               static_cast<double>(zlp::PFreq::kRange.end)));
        background_panel_.updateSampleRate(sample_rate);
        response_panel_.updateSampleRate(sample_rate);
        piano_roll_panel_.updateSampleRate(sample_rate);
        resized();
    }

    void CurvePanel::updatePianoRollVisibility() {
        const auto visible = piano_roll_panel_.isPianoRollVisible();
        if (piano_roll_visible_ != visible) {
            piano_roll_visible_ = visible;
            background_panel_.setFrequencyLabelsVisible(!visible);
            updateValueNotePanel();
        }
    }

    void CurvePanel::updateValueNotePanel(const bool force) {
        ValueNoteState state;
        state.show_values = value_display_on_ref_.load(std::memory_order::relaxed) > .5f;
        state.piano_visible = piano_roll_panel_.isPianoRollVisible();
        if (state.show_values) {
            const auto source = juce::Desktop::getInstance().getMainMouseSource();
            const auto* component = source.getComponentUnderMouse();
            if (component != nullptr && (component == this || isParentOf(component))) {
                state.dragging = source.isDragging()
                    && (piano_roll_panel_.getDraggedBandValues(state.frequency, state.gain)
                        || response_panel_.getDraggedBandValues(component, state.frequency, state.gain));
                if (state.dragging) {
                    state.mouse_over = true;
                } else {
                    state.position = getLocalPoint(nullptr, source.getScreenPosition());
                    state.mouse_over = response_panel_.getBounds().toFloat().contains(state.position) && fft_max_ > 10.0;
                }
            }
        }
        const auto over_piano = !state.dragging && state.mouse_over && state.piano_visible
            && piano_roll_panel_.getBounds().toFloat().contains(state.position);
        if (state.piano_visible && (!state.show_values || over_piano)) {
            state.note = piano_roll_panel_.getDisplayNote();
            if (state.show_values) {
                state.frequency = piano_roll_panel_.getDisplayFrequency();
            }
        } else if (state.mouse_over && !state.dragging) {
            const auto db_idx = static_cast<size_t>(std::clamp(
                static_cast<int>(std::round(eq_max_db_idx_ref_.load(std::memory_order::relaxed))),
                0, static_cast<int>(zlstate::PEQMaxDB::kChoices.size() - 1)));
            state.max_db = base_.getCurveDBScale(db_idx);
        }
        if (!force && value_note_state_ == state) {
            return;
        }
        value_note_state_ = state;

        value_note_panel_.setVisible(state.show_values ? state.mouse_over : state.piano_visible);
        if (!value_note_panel_.isVisible()) {
            return;
        }

        auto mode = state.show_values ? ValueNotePanel::Mode::kFrequencyGain : ValueNotePanel::Mode::kNote;
        auto frequency = state.frequency;
        auto gain = state.gain;
        if (state.show_values && !state.dragging) {
            mode = over_piano ? ValueNotePanel::Mode::kFrequencyNote : ValueNotePanel::Mode::kFrequencyGain;
            if (!over_piano || frequency <= 0.) {
                frequency = piano_roll_helper::xToFrequency(state.position.x, static_cast<float>(getWidth()), fft_max_);
            }
            frequency = std::clamp(frequency, 10.0, frequency_max_);
            if (!over_piano) {
                const auto font_size = base_.getFontSize();
                const auto plot_height = static_cast<float>(response_panel_.getHeight() - getBottomAreaHeight(font_size));
                const auto plot_y = state.position.y - static_cast<float>(response_panel_.getY());
                const auto padding = font_size * kDraggerScale;
                gain = std::clamp((plot_height - 2.f * plot_y)
                                     / std::max(plot_height - 2.f * padding, 1.f), -1.f, 1.f) * state.max_db;
            }
        }
        value_note_panel_.setValues(mode, state.piano_visible, frequency, gain, state.note);
    }

    void CurvePanel::startThreads() {
        startThread(juce::Thread::Priority::low);
        response_panel_.startThread(juce::Thread::Priority::low);
    }

    void CurvePanel::stopThreads() {
        if (isThreadRunning()) {
            stopThread(-1);
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
            is_match_on_.store(idx > 0, std::memory_order::relaxed);
            scale_panel_.setVisible(idx > 0);
            fft_panel_.setVisible(idx == 0);
            response_panel_.setVisible(idx != 1 && idx != 2);
            response_panel_.turnMatchON(idx > 0);
        }
    }
}
