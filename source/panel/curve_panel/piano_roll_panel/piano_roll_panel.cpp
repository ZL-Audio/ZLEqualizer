// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "piano_roll_panel.hpp"
#include "../../helper/band_helper.hpp"
#include "../../helper/freq_helper.hpp"
#include "../../helper/piano_roll_helper.hpp"

namespace zlpanel {
    PianoRollPanel::PianoRollPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        piano_roll_on_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PPianoRollON::kID)),
        keys_panel_(base), bands_panel_(base, keys_panel_), note_panel_(base), toggle_panel_(p, base) {
        band_notes_.fill(-1);
        for (size_t band = 0; band < bands_.size(); ++band) {
            const auto suffix = std::to_string(band);
            bands_[band].parameter = p.parameters_.getParameter(zlp::PFreq::kID + suffix);
            bands_[band].frequency = p.parameters_.getRawParameterValue(zlp::PFreq::kID + suffix);
            bands_[band].status = p.parameters_.getRawParameterValue(zlp::PFilterStatus::kID + suffix);
        }
        addChildComponent(keys_panel_);
        addChildComponent(bands_panel_);
        addChildComponent(note_panel_);
        toggle_panel_.setAlwaysOnTop(true);
        addAndMakeVisible(toggle_panel_);
        repaintCallBack();
    }

    PianoRollPanel::~PianoRollPanel() {
        endGesture();
    }

    void PianoRollPanel::resized() {
        const auto bound = getLocalBounds();
        keys_panel_.setBounds(bound);
        bands_panel_.setBounds(bound);
        const auto edge_width = std::min(bound.getWidth(), std::max(
            static_cast<int>(std::round(base_.getFontSize() * 3.75f)),
            bound.getHeight() + bound.getHeight() / 2));
        note_panel_.setBounds(bound.withLeft(bound.getRight() - edge_width));
        toggle_panel_.setBounds(bound.withWidth(edge_width));
        endGesture();
        updateBands(true, true);
    }

    bool PianoRollPanel::hitTest(const int x, const int y) {
        return toggle_panel_.getBounds().contains(x, y)
            || (piano_roll_visible_ && getLocalBounds().contains(x, y));
    }

    void PianoRollPanel::updateBand() {
        if (!piano_roll_visible_) {
            endGesture();
            return;
        }
        const auto selected = base_.getSelectedBand();
        if (drag_band_ < bands_.size() && drag_band_ != selected) {
            endGesture();
        }
        if (selected < bands_.size()) {
            updateSingleBand(selected);
        }
        bands_panel_.setSelectedBand(selected);
        updateNote();
    }

    void PianoRollPanel::updateSampleRate(const double sample_rate) {
        endGesture();
        slider_max_ = std::min(freq_helper::getSliderMax(sample_rate), static_cast<double>(zlp::PFreq::kRange.end));
        first_note_ = static_cast<int>(std::ceil(piano_roll_helper::frequencyToNote(piano_roll_helper::kMinFrequency)));
        last_note_ = static_cast<int>(std::floor(piano_roll_helper::frequencyToNote(slider_max_)));
        keys_panel_.updateSampleRate(sample_rate);
        updateBands(true, true);
    }

    void PianoRollPanel::repaintCallBack() {
        toggle_panel_.repaintCallBack();
        const auto visible = piano_roll_on_ref_.load(std::memory_order::relaxed) > .5f;
        const auto changed = piano_roll_visible_ != visible;
        if (changed) {
            piano_roll_visible_ = visible;
            keys_panel_.setVisible(visible);
            bands_panel_.setVisible(visible);
            note_panel_.setVisible(visible);
            hovered_note_ = -1;
        }
        updateBands(changed, changed);
    }

    void PianoRollPanel::updateBands(const bool force, const bool geometry_changed) {
        if (!piano_roll_visible_) {
            endGesture();
            return;
        }
        // poll existing atomics on the message thread; unchanged bands need no geometry or repaint work
        for (size_t band = 0; band < bands_.size(); ++band) {
            updateBandState(band, force);
        }
        const auto selected = base_.getSelectedBand();
        if (drag_band_ < bands_.size() && (drag_band_ != selected || band_notes_[drag_band_] < 0)) {
            endGesture();
        }
        bands_panel_.setBands(band_notes_, selected, geometry_changed);
        updateNote();
    }

    void PianoRollPanel::updateBandState(const size_t band, const bool force) {
        auto& state = bands_[band];
        if (state.status->load(std::memory_order::relaxed) <= .5f || last_note_ < first_note_) {
            band_notes_[band] = -1;
            return;
        }
        const auto frequency = state.frequency->load(std::memory_order::relaxed);
        if (!std::isfinite(frequency)) {
            band_notes_[band] = -1;
            return;
        }
        if (force || !juce::exactlyEqual(frequency, state.cached_frequency) || band_notes_[band] < 0) {
            state.cached_frequency = frequency;
            const auto clamped = std::clamp(static_cast<double>(frequency), piano_roll_helper::kMinFrequency,
                                            slider_max_);
            band_notes_[band] = std::clamp(
                static_cast<int>(std::round(piano_roll_helper::frequencyToNote(clamped))),
                first_note_, last_note_);
        }
    }

    void PianoRollPanel::updateSingleBand(const size_t band) {
        updateBandState(band);
        if (drag_band_ == band && (band != base_.getSelectedBand() || band_notes_[band] < 0)) {
            endGesture();
        }
        bands_panel_.setBand(band, band_notes_[band]);
    }

    void PianoRollPanel::updateHover(const juce::Point<float> point) {
        hovered_note_ = piano_roll_visible_ ? keys_panel_.getNoteAt(point) : -1;
        updateNote();
    }

    void PianoRollPanel::updateNote() {
        auto note = hovered_note_;
        if (drag_band_ < bands_.size()) {
            note = band_notes_[drag_band_];
        } else if (note < 0 && base_.getSelectedBand() < bands_.size()) {
            note = band_notes_[base_.getSelectedBand()];
        }
        note_panel_.setNote(note);
    }

    void PianoRollPanel::endGesture() {
        if (drag_band_ < bands_.size()) {
            bands_[drag_band_].parameter->endChangeGesture();
            drag_band_ = bands_.size();
        }
    }

    void PianoRollPanel::lookAndFeelChanged() {
        keys_panel_.resized();
        resized();
        bands_panel_.repaint();
    }

    void PianoRollPanel::visibilityChanged() {
        if (!isShowing()) {
            endGesture();
            hovered_note_ = -1;
        }
    }

    void PianoRollPanel::mouseDown(const juce::MouseEvent& event) {
        endGesture();
        if (!piano_roll_visible_ || !event.mods.isLeftButtonDown()) {
            return;
        }
        updateHover(event.position);
        const auto band = bands_panel_.getBandAt(event.position);
        if (band >= bands_.size()) {
            return;
        }
        updateSingleBand(band);
        if (band_notes_[band] < 0) {
            return;
        }
        base_.setSelectedBand(band);
        bands_panel_.setSelectedBand(band);
        if (event.getNumberOfClicks() < 2) {
            drag_band_ = band;
            const auto frequency = std::clamp(
                static_cast<double>(bands_[band].cached_frequency),
                piano_roll_helper::kMinFrequency, slider_max_);
            drag_x_ = keys_panel_.noteToX(piano_roll_helper::frequencyToNote(frequency));
            pointer_x_ = event.position.x;
            bands_[band].parameter->beginChangeGesture();
        }
        updateNote();
    }

    void PianoRollPanel::mouseDrag(const juce::MouseEvent& event) {
        if (drag_band_ >= bands_.size()) {
            return;
        }
        if (!piano_roll_visible_ || drag_band_ != base_.getSelectedBand()) {
            endGesture();
            updateBand();
            return;
        }
        const auto band = drag_band_;
        updateSingleBand(band);
        if (drag_band_ >= bands_.size()) {
            updateNote();
            return;
        }
        const auto shift = event.position.x - pointer_x_;
        pointer_x_ = event.position.x;
        if (event.mods.isCommandDown() || std::abs(shift) < 1e-7f) {
            updateNote();
            return;
        }
        const auto sensitivity = base_.getSensitivity(event.mods.isShiftDown()
            ? zlgui::kMouseDraggerFine
            : zlgui::kMouseDragger);
        const auto first_x = keys_panel_.noteToX(first_note_);
        const auto last_x = keys_panel_.noteToX(last_note_);
        // retain unsnapped motion so small and fine pointer movements accumulate
        drag_x_ = std::clamp(drag_x_ + shift * sensitivity, first_x, last_x);
        const auto note = std::clamp(static_cast<int>(std::round(keys_panel_.xToNote(drag_x_))), first_note_,
                                     last_note_);
        auto* parameter = bands_[band].parameter;
        const auto value = parameter->convertTo0to1(static_cast<float>(piano_roll_helper::noteToFrequency(note)));
        if (std::abs(value - parameter->getValue()) > 1e-7f) {
            parameter->setValueNotifyingHost(value);
            updateSingleBand(band);
        }
        updateNote();
    }

    void PianoRollPanel::mouseUp(const juce::MouseEvent& event) {
        endGesture();
        updateHover(event.position);
    }

    void PianoRollPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        endGesture();
        if (!piano_roll_visible_ || !event.mods.isLeftButtonDown()) {
            return;
        }
        const auto note = keys_panel_.getNoteAt(event.position);
        if (note < first_note_ || note > last_note_) {
            return;
        }
        const auto band = band_helper::findOffBand(p_ref_);
        if (band == zlp::kBandNum) {
            return;
        }
        const auto selected = base_.getSelectedBand();
        const auto lr_mode = selected < bands_.size()
            ? getValue(p_ref_.parameters_, zlp::PLRMode::kID + std::to_string(selected))
            : 0.f;
        const auto frequency = static_cast<float>(piano_roll_helper::noteToFrequency(note));
        const auto filter_type = band_helper::getInitialFilterType(frequency);
        const std::array ids{zlp::PFilterType::kID, zlp::PLRMode::kID, zlp::POrder::kID,
                             zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID, zlp::PDynamicON::kID};
        const std::array values{static_cast<float>(filter_type), lr_mode, 1.f, frequency, 0.f, .707f, 0.f};
        const auto suffix = std::to_string(band);
        for (size_t i = 0; i < ids.size(); ++i) {
            auto* parameter = p_ref_.parameters_.getParameter(ids[i] + suffix);
            updateValue(parameter, parameter->convertTo0to1(values[i]));
        }
        band_helper::turnOnOffDynamic(p_ref_, band, false, 0.f);
        // enable the recycled slot only after its frequency, gain and mode are ready
        auto* status = p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + suffix);
        updateValue(status, status->convertTo0to1(static_cast<float>(zlp::FilterStatus::kOn)));
        base_.setSelectedBand(band);
        updateSingleBand(band);
        bands_panel_.setSelectedBand(band);
        updateHover(event.position);
    }

    void PianoRollPanel::mouseEnter(const juce::MouseEvent& event) {
        updateHover(event.position);
    }

    void PianoRollPanel::mouseMove(const juce::MouseEvent& event) {
        updateHover(event.position);
    }

    void PianoRollPanel::mouseExit(const juce::MouseEvent&) {
        hovered_note_ = -1;
        updateNote();
    }
}
