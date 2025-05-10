// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "filter_button_panel.hpp"

namespace zlpanel {
    FilterButtonPanel::FilterButtonPanel(const size_t bandIdx, PluginProcessor &processor, zlgui::UIBase &base)
        : processor_ref_(processor),
          parameters_ref_(processor.parameters_), parameters_NA_ref_(processor.parameters_NA_),
          ui_base_(base),
          dragger_(base), target_dragger_(base), side_dragger_(base),
          button_pop_up_(bandIdx, parameters_ref_, parameters_NA_ref_, base),
          band_idx_{bandIdx},
          c_band_idx_(*parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)) {
        dragger_.addMouseListener(this, true);
        dragger_.getButton().setBufferedToImage(true);
        dragger_.setBroughtToFrontOnMouseClick(true);
        target_dragger_.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::upDownArrow);
        target_dragger_.setBroughtToFrontOnMouseClick(true);
        side_dragger_.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::rectangle);
        lookAndFeelChanged();
        for (const auto &idx: kIDs) {
            const auto idxD = zlp::appendSuffix(idx, band_idx_);
            parameters_ref_.addParameterListener(idxD, this);
            parameterChanged(idxD, parameters_ref_.getRawParameterValue(idxD)->load());
        }
        for (const auto &idx: kNAIDs) {
            const auto idx_s = zlstate::appendSuffix(idx, band_idx_);
            parameters_NA_ref_.addParameterListener(idx_s, this);
            parameterChanged(idx_s, parameters_NA_ref_.getRawParameterValue(idx_s)->load());
        }
        parameters_NA_ref_.addParameterListener(zlstate::selectedBandIdx::ID, this);
        parameterChanged(zlstate::selectedBandIdx::ID,
                         parameters_NA_ref_.getRawParameterValue(zlstate::selectedBandIdx::ID)->load());

        for (auto &d: {&side_dragger_, &target_dragger_, &dragger_}) {
            d->setScale(kScale);
            addAndMakeVisible(d);
        }
        addChildComponent(button_pop_up_);
        // set the current band if dragger is clicked
        dragger_.getButton().onClick = [this]() {
            if (dragger_.getButton().getToggleState()) {
                if (static_cast<size_t>(c_band_idx_.load()) != band_idx_) {
                    auto *para = parameters_NA_ref_.getParameter(zlstate::selectedBandIdx::ID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(zlstate::selectedBandIdx::convertTo01(static_cast<int>(band_idx_)));
                    para->endChangeGesture();
                }
                button_pop_up_.toFront(false);
                button_pop_up_.setVisible(true);
            } else {
                button_pop_up_.setVisible(false);
            }
        };
        // disable link if side dragger is clicked
        side_dragger_.getButton().onClick = [this]() {
            if (side_dragger_.getButton().getToggleState()) {
                const auto para = parameters_ref_.
                        getParameter(zlp::appendSuffix(zlp::singleDynLink::ID, band_idx_));
                para->beginChangeGesture();
                para->setValueNotifyingHost(0.f);
                para->endChangeGesture();
            }
        };

        setInterceptsMouseClicks(false, true);
        dragger_.setInterceptsMouseClicks(false, true);
        target_dragger_.setInterceptsMouseClicks(false, true);
        side_dragger_.setInterceptsMouseClicks(false, true);
    }

    FilterButtonPanel::~FilterButtonPanel() {
        for (const auto &idx: kIDs) {
            parameters_ref_.removeParameterListener(zlp::appendSuffix(idx, band_idx_), this);
        }
        for (const auto &idx: kNAIDs) {
            parameters_NA_ref_.removeParameterListener(zlstate::appendSuffix(idx, band_idx_), this);
        }
        parameters_NA_ref_.removeParameterListener(zlstate::selectedBandIdx::ID, this);
    }

    void FilterButtonPanel::resized() {
        updateBounds();
    }

    void FilterButtonPanel::setMaximumDB(const float db) {
        maximum_db_.store(db);
        to_update_attachment_.store(true);
        to_update_target_attachment_.store(true);
        to_update_draggers_.store(true);
    }

    void FilterButtonPanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id == zlstate::selectedBandIdx::ID) {
            is_selected_target_.store(static_cast<size_t>(new_value) == band_idx_);
            to_update_target_attachment_.store(true);
            to_update_draggers_.store(true);
            return;
        }
        if (parameter_id.startsWith(zlp::fType::ID)) {
            f_type_.store(static_cast<zldsp::filter::FilterType>(new_value));

            to_update_attachment_.store(true);
            to_update_target_attachment_.store(true);
            to_update_bounds_.store(true);
            to_update_draggers_.store(true);
        } else if (parameter_id.startsWith(zlstate::active::ID)) {
            const auto f = new_value > .5f;
            is_active_target_.store(f);
            to_update_target_attachment_.store(true);
            to_update_draggers_.store(true);
        } else if (parameter_id.startsWith(zlp::dynamicON::ID)) {
            is_dynamic_has_target_.store(new_value > .5f);
            to_update_target_attachment_.store(true);
            to_update_draggers_.store(true);
        } else if (parameter_id.startsWith(zlp::lrType::ID)) {
            lr_type_.store(static_cast<zlp::lrType::lrTypes>(new_value));
            to_update_dragger_label_.store(true);
            to_update_draggers_.store(true);
        }
    }

    void FilterButtonPanel::handleAsyncUpdate() {
        const auto f = is_active_target_.load();
        setVisible(f);
        dragger_.setVisible(f);
        dragger_.getButton().setToggleState(is_selected_target_.load(), juce::sendNotificationSync);
        if (to_update_attachment_.exchange(false)) {
            updateAttachment();
        }
        if (to_update_target_attachment_.exchange(false)) {
            updateTargetAttachment();
        }
        if (to_update_dragger_label_.exchange(false)) {
            updateDraggerLabel();
        }
        if (to_update_bounds_.exchange(false)) {
            updateBounds();
        }
        dragger_.getButton().repaint();
    }

    void FilterButtonPanel::updateAttachment() {
        const auto c_maximum_db = maximum_db_.load();
        const auto gain_range = juce::NormalisableRange<float>(-c_maximum_db, c_maximum_db, .01f);
        switch (f_type_.load()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf:
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                auto *para1 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::freq::ID, band_idx_));
                auto *para2 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::gain::ID, band_idx_));
                base_attach_ = std::make_unique<zlgui::DraggerParameterAttach>(
                    *para1, kFreqRange,
                    *para2, gain_range,
                    dragger_);
                base_attach_->enableX(true);
                base_attach_->enableY(true);
                base_attach_->sendInitialUpdate();
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                auto *para1 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::freq::ID, band_idx_));
                auto *para2 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::gain::ID, band_idx_));
                base_attach_ = std::make_unique<zlgui::DraggerParameterAttach>(
                    *para1, kFreqRange,
                    *para2, gain_range,
                    dragger_);
                base_attach_->enableX(true);
                base_attach_->enableY(false); {
                    base_attach_->setY(0.5f);
                }
                base_attach_->sendInitialUpdate();
                break;
            }
        }
    }

    void FilterButtonPanel::updateBounds() {
        dragger_.setBounds(getLocalBounds());
        target_dragger_.setBounds(getLocalBounds());
        side_dragger_.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight((1 - 0.98761596f) * bound.getWidth());
        switch (f_type_.load()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() - 2 * ui_base_.getFontSize()));
                target_dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() - 2 * ui_base_.getFontSize()));
                break;
            }
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() * .5f - 1 * ui_base_.getFontSize()));
                target_dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        bound.getHeight() * .5f - 1 * ui_base_.getFontSize()));
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(),
                        kScale * ui_base_.getFontSize()));
                break;
            }
        }
        juce::Rectangle<float> side_bound{
            bound.getX(), bound.getBottom() - 2 * ui_base_.getFontSize() - .5f * kScale * ui_base_.getFontSize(),
            bound.getWidth(), kScale * ui_base_.getFontSize()
        };
        side_bound = side_bound.withSizeKeepingCentre(side_bound.getWidth(), 1.f);
        side_dragger_.setButtonArea(side_bound);
    }

    void FilterButtonPanel::updateTargetAttachment() {
        bool is_filter_type_has_target = false;
        switch (f_type_.load()) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf:
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                is_filter_type_has_target = true;
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                is_filter_type_has_target = false;
                break;
            }
        }
        if (is_dynamic_has_target_.load() && is_filter_type_has_target &&
            is_selected_target_.load() && is_active_target_.load()) {
            const auto maxDB = maximum_db_.load();
            const auto gainRange = juce::NormalisableRange<float>(-maxDB, maxDB, .01f);
            auto *para1 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::freq::ID, band_idx_));
            auto *para3 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::targetGain::ID, band_idx_));
            target_attach_ = std::make_unique<zlgui::DraggerParameterAttach>(
                *para1, kFreqRange,
                *para3, gainRange,
                target_dragger_);
            target_attach_->enableX(true);
            target_attach_->enableY(true);
            target_attach_->sendInitialUpdate();
            target_dragger_.setVisible(true);
        } else {
            target_attach_.reset();
            target_dragger_.setVisible(false);
        }
        if (is_dynamic_has_target_.load() && is_selected_target_.load() && is_active_target_.load()) {
            const auto c_maximum_db = maximum_db_.load();
            const auto gain_range = juce::NormalisableRange<float>(-c_maximum_db, c_maximum_db, .01f);
            auto *para2 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::sideFreq::ID, band_idx_));
            auto *para3 = parameters_ref_.getParameter(zlp::appendSuffix(zlp::targetGain::ID, band_idx_));
            side_attach_ = std::make_unique<zlgui::DraggerParameterAttach>(
                *para2, kFreqRange,
                *para3, gain_range,
                side_dragger_);
            side_attach_->enableX(true);
            side_attach_->enableY(false);
            side_attach_->sendInitialUpdate();
            side_dragger_.setVisible(true);
        } else {
            side_attach_.reset();
            side_dragger_.setVisible(false);
        }
    }

    void FilterButtonPanel::updateDraggerLabel() {
        switch (lr_type_.load()) {
            case zlp::lrType::kStereo:
                dragger_.getLAF().setLabel("");
            break;
            case zlp::lrType::kLeft:
                dragger_.getLAF().setLabel("L");
            break;
            case zlp::lrType::kRight:
                dragger_.getLAF().setLabel("R");
            break;
            case zlp::lrType::kMid:
                dragger_.getLAF().setLabel("M");
            break;
            case zlp::lrType::kSide:
                dragger_.getLAF().setLabel("S");
            break;
        }
    }

    void FilterButtonPanel::setSelected(const bool f) {
        if (dragger_.getButton().getToggleState() != f) {
            dragger_.getButton().setToggleState(f, juce::NotificationType::sendNotificationSync);
        }
        if (target_dragger_.getButton().getToggleState()) {
            target_dragger_.getButton().setToggleState(false, juce::NotificationType::dontSendNotification);
        }
        if (side_dragger_.getButton().getToggleState()) {
            side_dragger_.getButton().setToggleState(false, juce::NotificationType::dontSendNotification);
        }
    }

    void FilterButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.mods.isCommandDown()) {
            const auto c_band_idx = band_idx_;
            if (event.mods.isLeftButtonDown()) {
                // turn on/off current band dynamic
                const auto parameter_id = zlp::appendSuffix(zlp::dynamicON::ID, c_band_idx);
                const auto new_value = 1.f - parameters_ref_.getRawParameterValue(parameter_id)->load(); {
                    auto *para = parameters_ref_.getParameter(parameter_id);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(new_value);
                    para->endChangeGesture();
                }
                float dyn_link_value = 0.0;
                if (new_value > 0.5f) {
                    processor_ref_.getFiltersAttach().turnOnDynamic(c_band_idx);
                    dyn_link_value = static_cast<float>(ui_base_.getDynLink());
                } else {
                    processor_ref_.getFiltersAttach().turnOffDynamic(c_band_idx);
                } {
                    auto *para = parameters_ref_.getParameter(
                        zlp::appendSuffix(zlp::singleDynLink::ID, c_band_idx));
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(dyn_link_value);
                    para->endChangeGesture();
                }
            } else if (event.mods.isRightButtonDown()) {
                auto *para = parameters_ref_.getParameter(
                    zlp::appendSuffix(zlp::solo::ID, c_band_idx));
                para->beginChangeGesture();
                if (para->getValue() < 0.5f) {
                    para->setValueNotifyingHost(1.f);
                } else {
                    para->setValueNotifyingHost(0.f);
                }
                para->endChangeGesture();
            }
        }
    }

    void FilterButtonPanel::lookAndFeelChanged() {
        for (auto &d: {&side_dragger_, &target_dragger_, &dragger_}) {
            d->getLAF().setColour(ui_base_.getColorMap1(band_idx_));
        }
    }

    void FilterButtonPanel::visibilityChanged() {
        if (!isVisible()) {
            button_pop_up_.setBounds({
                std::numeric_limits<int>::min() / 2, std::numeric_limits<int>::min() / 2,
                button_pop_up_.getBounds().getWidth(), button_pop_up_.getBounds().getHeight()
            });
        }
    }
} // zlpanel
