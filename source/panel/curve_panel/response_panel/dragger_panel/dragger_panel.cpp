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
                               const multilingual::TooltipHelper& tooltip_helper,
                               RightClickPanel& right_click_panel) :
        p_ref_(p), base_(base),
        mouse_event_panel_(p, base, tooltip_helper, right_click_panel),
        lasso_band_updater_(p, base),
        right_click_panel_(right_click_panel),
        items_set_(base.getSelectedBandSet()),
        draggers_(make_dragger_array(base, std::make_index_sequence<zlp::kBandNum>())),
        target_dragger_(base),
        side_dragger_(base),
        float_pop_panel_(p, base, tooltip_helper),
        max_db_id_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)),
        q_slider_(base), slope_slider_(base) {
        mouse_event_panel_.addMouseListener(this, false);
        addAndMakeVisible(mouse_event_panel_);

        side_dragger_.setScale(kDraggerScale * kDraggerSizeMultiplier);
        side_dragger_.getButton().setToggleState(true, juce::sendNotificationSync);
        side_dragger_.getButton().addMouseListener(this, false);
        side_dragger_.setXYEnabled(true, false);
        side_dragger_.getLAF().setDraggerShape(zlgui::dragger::DraggerLookAndFeel::kRectangle);
        addChildComponent(side_dragger_);

        target_dragger_.setScale(kDraggerScale * kDraggerSizeMultiplier);
        target_dragger_.getButton().setToggleState(true, juce::sendNotificationSync);
        target_dragger_.getButton().addMouseListener(this, false);
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

        float_pop_panel_.setBufferedToImage(true);
        addChildComponent(float_pop_panel_);

        addChildComponent(lasso_component_);
        items_set_.addChangeListener(this);

        base_.getSoloWholeIdxTree().addListener(this);

        lookAndFeelChanged();
        setInterceptsMouseClicks(false, true);
    }

    DraggerPanel::~DraggerPanel() {
        base_.getSoloWholeIdxTree().removeListener(this);
    }

    void DraggerPanel::resized() {
        bound_ = getLocalBounds().toFloat();
        mouse_event_panel_.setBounds(getLocalBounds());
        target_dragger_.setBounds(getLocalBounds());
        side_dragger_.setBounds(getLocalBounds());
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            draggers_[band].setBounds(getLocalBounds());
            updateDraggerBound(band);
        }
        const auto float_width = float_pop_panel_.getIdealWidth();
        const auto float_height = float_pop_panel_.getIdealHeight();
        float_pop_panel_.setBounds({0, 0, float_width, float_height});

        auto bound = getLocalBounds().toFloat();
        bound.removeFromBottom(static_cast<float>(getBottomAreaHeight(base_.getFontSize())));
        float_pop_panel_.updateFloatingBound(bound);
    }

    void DraggerPanel::repaintCallBack() {
        lasso_band_updater_.repaintCallBack();
    }

    void DraggerPanel::repaintCallBackSlow() {
        mouse_event_panel_.repaintCallbackSlow();
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
        if (ftype_idx_ref_ && !is_slope_attach_side_) {
            const auto ftype_idx = ftype_idx_ref_->load(std::memory_order::relaxed);
            if (std::abs(ftype_idx - c_ftype_idx_) > .1f) {
                c_ftype_idx_ = ftype_idx;
                updateSlopeAttachment();
            }
        }
        if (side_ftype_idx_ref_ && is_slope_attach_side_) {
            const auto side_ftype_idx = side_ftype_idx_ref_->load(std::memory_order::relaxed);
            if (std::abs(side_ftype_idx - c_side_ftype_idx_) > .1f) {
                c_side_ftype_idx_ = side_ftype_idx;
                updateSlopeAttachment();
            }
        }
        float_pop_panel_.repaintCallBackSlow();
        updater_.updateComponents();
    }

    void DraggerPanel::updateBand() {
        mouse_event_panel_.updateBand();
        lasso_band_updater_.updateBand();
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (band != base_.getSelectedBand()) {
                draggers_[band].getButton().setToggleState(false, juce::sendNotificationSync);
            }
        }
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            draggers_[band].toFront(true);
            target_dragger_.setVisible(is_dynamic_on_[band]);
            side_dragger_.setVisible(is_dynamic_on_[band]);

            target_dragger_.getLAF().setColour(base_.getColourMap1(band));
            side_dragger_.getLAF().setColour(base_.getColourMap1(band));

            updateDraggerBound(band);
            updateDraggerAttachment(band);
            updateTargetAttachment(band);
            updateSideAttachment(band);

            if (!items_set_.isSelected(band)) {
                items_set_.deselectAll();
            }
        } else {
            target_dragger_.setVisible(false);
            side_dragger_.setVisible(false);
        }
        float_pop_panel_.updateBand();
    }

    void DraggerPanel::updateSampleRate(const double sample_rate) {
        mouse_event_panel_.updateSampleRate(sample_rate);
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
        draggers_[band].setAlpha(filter_status == zlp::FilterStatus::kBypass ? kBypassAlphaMultiplier : 1.f);
        draggers_[band].getLAF().setAlpha(is_same_stereo ? 1.f : kDiffStereoAlphaMultiplier);
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
        lasso_component_.setColour(juce::LassoComponent<size_t>::lassoFillColourId,
                                   base_.getTextColour().withMultipliedAlpha(.125f));
        lasso_component_.setColour(juce::LassoComponent<size_t>::lassoOutlineColourId,
                                   base_.getTextColour().withMultipliedAlpha(.15f));
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
            dragger_y_enabled_[band] = true;
            break;
        }
        case zldsp::filter::kLowShelf:
        case zldsp::filter::kHighShelf:
        case zldsp::filter::kTiltShelf: {
            draggers_[band].setXYEnabled(true, true);
            dragger_y_enabled_[band] = true;
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() * .5f);
            break;
        }
        case zldsp::filter::kLowPass:
        case zldsp::filter::kHighPass:
        case zldsp::filter::kBandPass:
        case zldsp::filter::kNotch:
        default: {
            draggers_[band].setXYEnabled(true, false);
            dragger_y_enabled_[band] = false;
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

    void DraggerPanel::updateSlopeAttachment() {
        if (slope_attach_band_ == zlp::kBandNum) {
            slope_attachment_.reset();
            return;
        }
        const auto band_s = std::to_string(slope_attach_band_);
        ftype_idx_ref_ = p_ref_.parameters_.getRawParameterValue(zlp::PFilterType::kID + band_s);
        side_ftype_idx_ref_ = p_ref_.parameters_.getRawParameterValue(zlp::PSideFilterType::kID + band_s);
        if (!is_slope_attach_side_) {
            const auto ftype = static_cast<int>(c_ftype_idx_);
            const auto slope_6_disabled = (ftype == static_cast<int>(zldsp::filter::kPeak))
                || (ftype == static_cast<int>(zldsp::filter::kBandPass))
                || (ftype == static_cast<int>(zldsp::filter::kNotch));
            slope_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                slope_slider_, p_ref_.parameters_, zlp::POrder::kID + band_s,
                juce::NormalisableRange<double>(slope_6_disabled ? 1.0 : 0.0, 6.0, 1.0),
                updater_);
        } else {
            const auto slope_6_disabled = c_side_ftype_idx_ < .5f;
            slope_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                slope_slider_, p_ref_.parameters_, zlp::PSideOrder::kID + band_s,
                juce::NormalisableRange<double>(slope_6_disabled ? 1.0 : 0.0, 6.0, 1.0),
                updater_);
        }
        slope_attachment_->updateComponent();
    }

    void DraggerPanel::mouseDown(const juce::MouseEvent& event) {
        if (event.originalComponent == &mouse_event_panel_) {
            items_set_.deselectAll();
            lasso_component_.setVisible(true);
            lasso_component_.beginLasso(event, this);
            return;
        }
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &(side_dragger_.getButton())) {
                updateValue(p_ref_.parameters_.getParameter(zlp::PSideLink::kID + std::to_string(band)), 0.f);
            }
            if (event.originalComponent == &(draggers_[band].getButton())
                || event.originalComponent == &(target_dragger_.getButton())) {
                const auto solo_whole_idx = base_.getSoloWholeIdx();
                if (solo_whole_idx > zlp::kBandNum && solo_whole_idx < 2 * zlp::kBandNum) {
                    base_.setSoloWholeIdx(2 * zlp::kBandNum);
                }
            }
            if (event.originalComponent == &(draggers_[band].getButton())) {
                if (event.mods.isRightButtonDown() && !event.mods.isCommandDown()) {
                    right_click_panel_.setPosition(draggers_[band].getButtonPos());
                    right_click_panel_.setVisible(true);
                    return;
                }
            }
        }
        right_click_panel_.setVisible(false);
    }

    void DraggerPanel::mouseUp(const juce::MouseEvent& event) {
        if (event.originalComponent == &mouse_event_panel_) {
            lasso_component_.endLasso();
            lasso_component_.setVisible(false);
            lasso_band_updater_.loadParas();
        }
    }

    void DraggerPanel::mouseDrag(const juce::MouseEvent& event) {
        if (event.originalComponent == &mouse_event_panel_) {
            lasso_component_.dragLasso(event);
        }
    }

    void DraggerPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &(draggers_[band].getButton())) {
                if (event.mods.isCommandDown()) {
                    if (event.mods.isLeftButtonDown()) {
                        const auto dynamic_on = getValue(
                            p_ref_.parameters_, zlp::PDynamicON::kID + std::to_string(band)) > .5f;
                        updateValue(p_ref_.parameters_.getParameter(zlp::PDynamicON::kID + std::to_string(band)),
                                    dynamic_on ? 0.f : 1.f);
                        band_helper::turnOnOffDynamic(p_ref_, band, !dynamic_on);
                    }
                } else {
                    if (event.mods.isLeftButtonDown()) {
                        if (base_.getSoloWholeIdx() < zlp::kBandNum) {
                            base_.setSoloWholeIdx(2 * zlp::kBandNum);
                        } else {
                            base_.setSoloWholeIdx(band);
                        }
                    }
                }
            } else if (event.originalComponent == &(side_dragger_.getButton())) {
                if (base_.getSoloWholeIdx() == 2 * zlp::kBandNum) {
                    base_.setSoloWholeIdx(zlp::kBandNum + band);
                } else {
                    base_.setSoloWholeIdx(2 * zlp::kBandNum);
                }
            }
        }
    }

    void DraggerPanel::mouseEnter(const juce::MouseEvent& event) {
        if (const auto band = base_.getSelectedBand(); band < zlp::kBandNum) {
            if (event.originalComponent == &(draggers_[band].getButton())
                || event.originalComponent == &(target_dragger_.getButton())) {
                q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                    q_slider_, p_ref_.parameters_, zlp::PQ::kID + std::to_string(band), updater_);
                q_attachment_->updateComponent();
                slope_attach_band_ = band;
                is_slope_attach_side_ = false;
                updateSlopeAttachment();
                return;
            }
            if (event.originalComponent == &(side_dragger_.getButton())) {
                q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                    q_slider_, p_ref_.parameters_, zlp::PSideQ::kID + std::to_string(band), updater_);
                q_attachment_->updateComponent();
                slope_attach_band_ = band;
                is_slope_attach_side_ = true;
                updateSlopeAttachment();
                return;
            }
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (event.originalComponent == &(draggers_[band].getButton())) {
                q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                    q_slider_, p_ref_.parameters_, zlp::PQ::kID + std::to_string(band), updater_);
                q_attachment_->updateComponent();
                slope_attach_band_ = band;
                is_slope_attach_side_ = false;
                updateSlopeAttachment();
                return;
            }
        }
    }

    void DraggerPanel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        if (event.mods.isCommandDown()) {
            slope_slider_.mouseWheelMove(event, wheel);
        } else {
            q_slider_.mouseWheelMove(event, wheel);
        }
    }

    void DraggerPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) {
        const auto solo_whole_idx = base_.getSoloWholeIdx();
        if (previous_solo_whole_idx_ < zlp::kBandNum) {
            draggers_[previous_solo_whole_idx_].setXYEnabled(
                true, dragger_y_enabled_[previous_solo_whole_idx_]);
        }
        if (solo_whole_idx < zlp::kBandNum) {
            draggers_[solo_whole_idx].setXYEnabled(true, false);
        }
        previous_solo_whole_idx_ = solo_whole_idx;
    }

    void DraggerPanel::findLassoItemsInArea(juce::Array<size_t>& items_found, const juce::Rectangle<int>& area) {
        const auto float_area = area.toFloat();
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (draggers_[band].isVisible()) {
                if (float_area.contains(draggers_[band].getButtonPos())) {
                    items_found.add(band);
                }
            }
        }
    }

    juce::SelectedItemSet<size_t>& DraggerPanel::getLassoSelection() {
        return items_set_;
    }

    void DraggerPanel::changeListenerCallback(juce::ChangeBroadcaster*) {
        if (items_set_.getNumSelected() == 1) {
            base_.setSelectedBand(items_set_.getSelectedItem(0));
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto f1 = items_set_.isSelected(band);
            if (f1 != draggers_[band].getLAF().getIsSelected()) {
                draggers_[band].getLAF().setIsSelected(f1);
                draggers_[band].getButton().repaint();
            }
        }
    }
}
