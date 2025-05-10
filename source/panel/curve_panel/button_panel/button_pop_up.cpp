// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_pop_up.hpp"

namespace zlpanel {
    ButtonPopUp::PitchLabel::PitchLabel(zlgui::UIBase &base, juce::RangedAudioParameter *freq)
                : ui_base_(base), freq_para_(freq), laf_(ui_base_) {
        label_.setLookAndFeel(&laf_);
        label_.setJustificationType(juce::Justification::centredRight);
        laf_.setFontScale(1.2f);
        label_.setEditable(false, true);
        label_.addListener(this);
        addAndMakeVisible(label_);
        label_.setBufferedToImage(true);
    }

    void ButtonPopUp::PitchLabel::setFreq(const double freq) {
        const auto pitch_idx = juce::roundToInt(12 * std::log2(freq / 440.f));
        const auto pitch_idx1 = (pitch_idx + 240) % 12;
        const auto pitch_idx2 = (pitch_idx + 240) / 12 - 16;
        const auto pitch_string = pitch_idx2 >= 0
                                     ? std::string(kPitchLookUp[static_cast<size_t>(pitch_idx1)]) + std::to_string(
                                           pitch_idx2)
                                     : "A0";
        label_.setText(pitch_string, juce::dontSendNotification);
    }

    void ButtonPopUp::PitchLabel::editorShown(juce::Label *, juce::TextEditor &editor) {
        editor.setInputRestrictions(0, "0123456789.kKABCDEFGabcdefg#");

        editor.setJustification(juce::Justification::centredRight);
        editor.setColour(juce::TextEditor::outlineColourId, ui_base_.getTextColor());
        editor.setColour(juce::TextEditor::highlightedTextColourId, ui_base_.getTextColor());
        editor.applyFontToAllText(juce::FontOptions{ui_base_.getFontSize() * 1.2f});
        editor.applyColourToAllText(ui_base_.getTextColor(), true);

        editor.addListener(this);
        has_editor_changed_ = false;
    }

    void ButtonPopUp::PitchLabel::editorHidden(juce::Label *, juce::TextEditor &editor) {
        editor.removeListener(this);
        if (!has_editor_changed_) return;
        const auto s = editor.getText();
        double value;
        if (const auto v = parseFreqPitchString(s)) {
            value = v.value();
        } else {
            value = parseFreqValueString(s);
        }
        freq_para_->beginChangeGesture();
        freq_para_->setValueNotifyingHost(freq_para_->convertTo0to1(static_cast<float>(value)));
        freq_para_->endChangeGesture();

        setFreq(value);
    }

    void ButtonPopUp::PitchLabel::textEditorTextChanged(juce::TextEditor &) {
        has_editor_changed_ = true;
    }

    ButtonPopUp::ButtonPopUp(const size_t bandIdx, juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA, zlgui::UIBase &base)
        : band_idx_{bandIdx}, parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          ftype_(*parameters_ref_.getRawParameterValue(zlp::appendSuffix(zlp::fType::ID, band_idx_))),
          freq_para_(*parameters_ref_.getRawParameterValue(zlp::appendSuffix(zlp::freq::ID, band_idx_))),
          background_(bandIdx, parameters, parameters_NA, base),
          pitch_label_(base, parameters.getParameter(zlp::appendSuffix(zlp::freq::ID, bandIdx))) {
        juce::ignoreUnused(parameters_ref_, parameters_NA_ref_);

        addAndMakeVisible(background_);
        addAndMakeVisible(pitch_label_);
    }

    ButtonPopUp::~ButtonPopUp() {
        pitch_label_.setLookAndFeel(nullptr);
    }

    void ButtonPopUp::resized() {
        const auto current_bound = getLocalBounds();
        background_.setBounds(current_bound);

        auto bound = current_bound.toFloat();
        bound.removeFromBottom(bound.getHeight() * .4f);
        bound.removeFromLeft(bound.getWidth() * .705882f);
        bound.removeFromRight(ui_base_.getFontSize() * .25f);

        pitch_label_.setBounds(bound.toNearestInt());
    }

    void ButtonPopUp::updateBounds(const juce::Component &component) {
        if (getParentComponent() == nullptr || component.getParentComponent() == nullptr) {
            return;
        }
        const auto comp_bound = component.getBoundsInParent().toFloat();
        const auto comp_parent_bound = component.getParentComponent()->getLocalBounds().toFloat();
        const auto shift_x = comp_bound.getCentreX() - comp_parent_bound.getCentreX();
        const auto shift_y = comp_bound.getCentreY() - comp_parent_bound.getCentreY();
        const auto shift_y_portion = shift_y / (comp_parent_bound.getHeight() - ui_base_.getFontSize()) * 2;

        switch (static_cast<zldsp::filter::FilterType>(ftype_.load())) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                if (direction_ > 0.f) {
                    if (shift_y_portion > .5f || (shift_y_portion < -0.1f && shift_y_portion > -0.4f)) {
                        direction_ = -1.f;
                    }
                } else {
                    if (shift_y_portion < -0.5f || (shift_y_portion > 0.1f && shift_y_portion < 0.4f)) {
                        direction_ = 1.f;
                    }
                }
                break;
            }
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                if (direction_ > 0.f && shift_y_portion < -0.2f) {
                    direction_ = -1.f;
                } else if (direction_ < 0.f && shift_y_portion > 0.2f) {
                    direction_ = 1.f;
                }
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                direction_ = -1.f;
                break;
            }
        }

        const auto width = kWidthP * ui_base_.getFontSize();
        const auto height = kHeightP * ui_base_.getFontSize();
        const auto bound = getParentComponent()->getLocalBounds().toFloat();
        const auto final_y = bound.getCentreY() + direction_ * height + shift_y;
        const auto final_x = juce::jlimit(bound.getX() + width * .5f,
                                         bound.getRight() - width * .5f,
                                         bound.getCentreX() + shift_x);

        const auto pop_up_bound = juce::Rectangle<float>(width, height).withCentre({final_x, final_y});

        const auto actual_bound = juce::Rectangle<int>(
            juce::roundToInt(pop_up_bound.getX()), juce::roundToInt(pop_up_bound.getY()),
            juce::roundToInt(width), juce::roundToInt(height));

        if (actual_bound != previous_bound_) {
            previous_bound_ = actual_bound;
            updateLabel();
            setBounds(actual_bound);
        }
    }

    void ButtonPopUp::updateLabel() {
        pitch_label_.setFreq(freq_para_.load());
    }
} // zlpanel
