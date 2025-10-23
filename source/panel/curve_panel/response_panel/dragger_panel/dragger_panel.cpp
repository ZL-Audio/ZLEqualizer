// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_panel.hpp"

namespace zlpanel {
    DraggerPanel::DraggerPanel(PluginProcessor& p,
                               zlgui::UIBase& base,
                               const multilingual::TooltipHelper&) :
        p_ref_(p), base_(base),
        draggers_(make_dragger_array(base, std::make_index_sequence<zlp::kBandNum>())),
        target_dragger_(base),
        side_dragger_(base),
        max_db_id_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)) {

        side_dragger_.setScale(kDraggerScale * kDraggerSizeMultiplier);
        side_dragger_.getButton().setToggleState(true, juce::sendNotificationSync);
        side_dragger_.getButton().addMouseListener(this, false);
        side_dragger_.setXYEnabled(true, false);
        side_dragger_.getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kRectangle);
        addChildComponent(side_dragger_);

        target_dragger_.setScale(kDraggerScale * kDraggerSizeMultiplier);
        target_dragger_.getButton().setToggleState(true, juce::sendNotificationSync);
        target_dragger_.setXYEnabled(false, true);
        target_dragger_.getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kUpDownArrow);
        addChildComponent(target_dragger_);

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            draggers_[band].setBroughtToFrontOnMouseClick(true);
            draggers_[band].getButton().addMouseListener(this, false);
            draggers_[band].getButton().onClick = [this, band]() {
                if (draggers_[band].getButton().getToggleState()) {
                    base_.setSelectedBand(band);
                }
            };
            draggers_[band].setScale(kDraggerScale * kDraggerSizeMultiplier);
            draggers_[band].getButton().setBufferedToImage(true);
            draggers_[band].getLAF().setColour(base_.getColourMap1(band));
            draggers_[band].getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kRound);
            addChildComponent(draggers_[band]);
        }

        lookAndFeelChanged();
        setInterceptsMouseClicks(false, true);
    }

    void DraggerPanel::resized() {
        bound_ = getLocalBounds().toFloat();
        target_dragger_.setBounds(getLocalBounds());
        side_dragger_.setBounds(getLocalBounds());
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            draggers_[band].setBounds(getLocalBounds());
            updateDraggerBound(band);
        }
    }

    void DraggerPanel::repaintCallBackSlow() {
        const auto max_db_id = max_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(max_db_id - c_max_db_id_) > .1f) {
            c_max_db_id_ = max_db_id;
            const auto max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(c_max_db_id_))];
            gain_range_ = juce::NormalisableRange<float>(-max_db, max_db, .01f);
            if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
                updateDraggerAttachment(band);
                updateTargetAttachment(band);
            }
        }
    }

    void DraggerPanel::updateBand() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (band != base_.getSelectedBand()) {
                draggers_[band].getButton().setToggleState(false, juce::sendNotificationSync);
            }
        }
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            target_dragger_.setVisible(is_dynamic_on_[band]);
            side_dragger_.setVisible(is_dynamic_on_[band]);

            target_dragger_.getLAF().setColour(base_.getColourMap1(band));
            side_dragger_.getLAF().setColour(base_.getColourMap1(band));

            updateDraggerBound(band);
            updateDraggerAttachment(band);
            updateTargetAttachment(band);
            updateSideAttachment(band);
        } else {
            target_dragger_.setVisible(false);
            side_dragger_.setVisible(false);
        }
    }

    void DraggerPanel::updateSampleRate(const double sample_rate) {
        sample_rate_ = static_cast<float>(sample_rate);
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            updateDraggerBound(band);
        }
        const auto slider_max = freq_helper::getSliderMax(sample_rate);
        freq_range_ = juce::NormalisableRange<float>{
            10.f, static_cast<float>(slider_max),
            [](float range_start, float range_end, float value_to_remap) {
                return std::exp(value_to_remap * std::log(
                    range_end / range_start)) * range_start;
            },
            [](float range_start, float range_end, float value_to_remap) {
                return std::log(value_to_remap / range_start) / std::log(
                    range_end / range_start);
            },
            [](float range_start, float range_end, float value_to_remap) {
                return std::clamp(value_to_remap, range_start, range_end);
            }
        };
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            updateDraggerAttachment(band);
            updateSideAttachment(band);
        }
    }

    void DraggerPanel::updateFilterType(const size_t band, const zldsp::filter::FilterType filter_type) {
        if (filter_type != filter_types_[band]) {
            filter_types_[band] = filter_type;
            if (band == base_.getSelectedBand()) {
                updateDraggerBound(band);
            }
        }
    }

    void DraggerPanel::updateDrawingParas(const size_t band, const zlp::FilterStatus filter_status,
                                          const bool is_dynamic_on,
                                          const bool is_same_stereo, const int lr_mode) {
        draggers_[band].setVisible(filter_status != zlp::FilterStatus::kOff);
        if (band == base_.getSelectedBand()) {
            target_dragger_.setVisible(is_dynamic_on);
            side_dragger_.setVisible(is_dynamic_on);
        }
        is_dynamic_on_[band] = is_dynamic_on;
        float alpha = 1.f;
        if (filter_status == zlp::FilterStatus::kBypass) {
            alpha *= kBypassAlphaMultiplier;
        }
        if (!is_same_stereo) {
            alpha *= kDiffStereoAlphaMultiplier;
        }
        draggers_[band].getLAF().setAlpha(alpha);
        if (lr_mode == 0) {
            draggers_[band].getLAF().setLabel("");
        } else if (lr_mode == 1) {
            draggers_[band].getLAF().setLabel("L");
        } else if (lr_mode == 2) {
            draggers_[band].getLAF().setLabel("R");
        } else if (lr_mode == 3) {
            draggers_[band].getLAF().setLabel("M");
        } else if (lr_mode == 4) {
            draggers_[band].getLAF().setLabel("S");
        }
        draggers_[band].getButton().repaint();
    }

    void DraggerPanel::lookAndFeelChanged() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            draggers_[band].getLAF().setColour(base_.getColourMap1(band));
        }
    }

    void DraggerPanel::updateDraggerBound(const size_t band) {
        const auto fft_max = freq_helper::getFFTMax(sample_rate_);
        const auto slider_max = freq_helper::getSliderMax(sample_rate_);
        const auto width_p = (1.f - kFontSizeOverWidth * kDraggerScale) *
            static_cast<float>(std::log(slider_max * .1) / std::log(fft_max * .1));
        auto bound = getLocalBounds().toFloat();
        bound.removeFromTop(base_.getFontSize() * kDraggerScale);
        bound.removeFromBottom(static_cast<float>(getBottomAreaHeight(base_.getFontSize())));
        bound.setWidth(bound.getWidth() * width_p);
        const auto side_bound = bound.removeFromBottom(base_.getFontSize() * kDraggerScale);

        switch (filter_types_[band]) {
        case zldsp::filter::kPeak:
        case zldsp::filter::kBandShelf: {
            draggers_[band].setXYEnabled(true, true);
            break;
        }
        case zldsp::filter::kLowShelf:
        case zldsp::filter::kHighShelf:
        case zldsp::filter::kTiltShelf: {
            draggers_[band].setXYEnabled(true, true);
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() * .5f);
            break;
        }
        case zldsp::filter::kLowPass:
        case zldsp::filter::kHighPass:
        case zldsp::filter::kBandPass:
        case zldsp::filter::kNotch:
        default: {
            draggers_[band].setXYEnabled(true, false);
            break;
        }
        }
        draggers_[band].setButtonArea(bound);
        if (band == base_.getSelectedBand()) {
            target_dragger_.setButtonArea(bound);
            side_dragger_.setButtonArea(side_bound);
        }
    }

    void DraggerPanel::updateDraggerAttachment(const size_t band) {
        dragger_freq_attachment_.reset();
        dragger_freq_attachment_ = std::make_unique<zlgui::attachment::DraggerAttachment<false, true>>(
            draggers_[band], p_ref_.parameters_,
            zlp::PFreq::kID + std::to_string(band), freq_range_, updater_);
        dragger_gain_attachment_.reset();
        dragger_gain_attachment_ = std::make_unique<zlgui::attachment::DraggerAttachment<false, false>>(
            draggers_[band], p_ref_.parameters_,
            zlp::PGain::kID + std::to_string(band), gain_range_, updater_);
    }

    void DraggerPanel::updateTargetAttachment(const size_t band) {
        target_dragger_attachment_.reset();
        target_dragger_attachment_ = std::make_unique<zlgui::attachment::DraggerAttachment<false, false>>(
            target_dragger_, p_ref_.parameters_,
            zlp::PTargetGain::kID + std::to_string(band), gain_range_, updater_);
    }

    void DraggerPanel::updateSideAttachment(const size_t band) {
        side_dragger_attachment_.reset();
        side_dragger_attachment_ = std::make_unique<zlgui::attachment::DraggerAttachment<false, true>>(
            side_dragger_, p_ref_.parameters_,
            zlp::PSideFreq::kID + std::to_string(band), freq_range_, updater_);
    }

    void DraggerPanel::mouseDown(const juce::MouseEvent& event) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &(side_dragger_.getButton())) {
                updateValue(p_ref_.parameters_.getParameter(zlp::PSideLink::kID + std::to_string(band)), 0.f);
            }
        }
    }

    void DraggerPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &(draggers_[band].getButton())) {
                if (event.mods.isCommandDown()) {
                    const auto dynamic_on = getValue(p_ref_.parameters_, zlp::PDynamicON::kID + std::to_string(band)) >
                        .5f;
                    updateValue(p_ref_.parameters_.getParameter(zlp::PDynamicON::kID + std::to_string(band)),
                                dynamic_on ? 0.f : 1.f);
                    band_helper::turnOnOffDynamic(p_ref_, band, dynamic_on);
                }
            }
        }
    }
}
