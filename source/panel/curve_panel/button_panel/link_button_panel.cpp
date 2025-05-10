// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "link_button_panel.hpp"

namespace zlpanel {
    LinkButtonPanel::LinkButtonPanel(const size_t idx,
                                     juce::AudioProcessorValueTreeState &parameters,
                                     juce::AudioProcessorValueTreeState &parameters_NA,
                                     zlgui::UIBase &base,
                                     zlgui::Dragger &side_dragger)
        : parameters_ref_(parameters), parameters_NA_ref_(parameters_NA),
          ui_base_(base),
          side_dragger_ref_(side_dragger),
          dyn_link_c_("L", base),
          link_drawable_(juce::Drawable::createFromImageData(BinaryData::linksfill_svg, BinaryData::linksfill_svgSize)),
          band_idx_(idx) {
        dyn_link_c_.getLAF().enableShadow(false);
        dyn_link_c_.setDrawable(link_drawable_.get());
        attach({&dyn_link_c_.getButton()}, {zlp::appendSuffix(zlp::singleDynLink::ID, band_idx_)},
               parameters, button_attachments_);
        addAndMakeVisible(dyn_link_c_);
        side_dragger_ref_.addMouseListener(this, true);

        for (auto &ID: kIDs) {
            const auto suffixID = zlp::appendSuffix(ID, idx);
            parameters_ref_.addParameterListener(suffixID, this);
            parameterChanged(suffixID, parameters_ref_.getRawParameterValue(suffixID)->load());
        }
        for (auto &ID: kNAIDs) {
            parameters_NA_ref_.addParameterListener(ID, this);
            parameterChanged(ID, parameters_NA_ref_.getRawParameterValue(ID)->load());
        }
        setInterceptsMouseClicks(false, true);
    }

    LinkButtonPanel::~LinkButtonPanel() {
        const auto idx = band_idx_.load();
        for (auto &ID: kIDs) {
            parameters_ref_.removeParameterListener(zlp::appendSuffix(ID, idx), this);
        }
        for (auto &ID: kNAIDs) {
            parameters_NA_ref_.removeParameterListener(ID, this);
        }
    }

    void LinkButtonPanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id.startsWith(zlp::dynamicON::ID)) {
            is_dynamic_on_.store(new_value > .5f);
        } else if (parameter_id.startsWith(zlstate::selectedBandIdx::ID)) {
            is_selected_.store(static_cast<size_t>(new_value) == band_idx_.load());
        }
    }

    void LinkButtonPanel::updateBound() {
        if (is_selected_.load() && is_dynamic_on_.load()) {
            const auto dyn_pos = static_cast<float>(side_dragger_ref_.getButton().getBoundsInParent().getCentreX());
            auto button_bound = juce::Rectangle<float>{button_size_, button_size_};
            button_bound = button_bound.withCentre({dyn_pos, button_bottom_});
            dyn_link_c_.setBounds(button_bound.toNearestInt());
            setVisible(true);
        } else {
            setVisible(false);
        }
    }

    void LinkButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.mods.isCommandDown() && event.mods.isRightButtonDown()) {
            const auto current_band = band_idx_.load();
            auto *para = parameters_ref_.getParameter(
                zlp::appendSuffix(zlp::sideSolo::ID, current_band));
            para->beginChangeGesture();
            if (para->getValue() < 0.5f) {
                para->setValueNotifyingHost(1.f);
            } else {
                para->setValueNotifyingHost(0.f);
            }
            para->endChangeGesture();
        }
    }

    void LinkButtonPanel::resized() {
        button_size_ = 2.5f * ui_base_.getFontSize();
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 8 * ui_base_.getFontSize());
        button_bottom_ = bound.getBottom();
    }
} // zlpanel
