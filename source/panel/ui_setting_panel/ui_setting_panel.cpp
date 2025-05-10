// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    UISettingPanel::UISettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : processor_ref_(p), ui_base_(base),
          colour_panel_(p, base),
          control_panel_(p, base),
          other_panel_(p, base),
          save_drawable_(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          close_drawable_(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          reset_drawable_(
              juce::Drawable::createFromImageData(BinaryData::loopleftline_svg, BinaryData::loopleftline_svgSize)),
          save_button_(ui_base_, save_drawable_.get()),
          close_button_(ui_base_, close_drawable_.get()),
          reset_button_(ui_base_, reset_drawable_.get()),
          panel_name_laf_(ui_base_),
          label_laf_(ui_base_) {
        juce::ignoreUnused(processor_ref_);
        setOpaque(true);
        ui_base_.setProperty(zlgui::SettingIdx::kUISettingPanelShow, false);
        addAndMakeVisible(save_button_);
        addAndMakeVisible(close_button_);
        addAndMakeVisible(reset_button_);
        view_port_.setScrollBarsShown(true, false,
                                    true, false);
        changeDisplayPanel();
        addAndMakeVisible(view_port_);
        save_button_.getButton().onClick = [this]() {
            switch (current_panel_idx_) {
                case kColourP: {
                    colour_panel_.saveSetting();
                    break;
                }
                case kControlP: {
                    control_panel_.saveSetting();
                    break;
                }
                case kOtherP: {
                    other_panel_.saveSetting();
                    break;
                }
            }
        };
        reset_button_.getButton().onClick = [this]() {
            switch (current_panel_idx_) {
                case kColourP: {
                    colour_panel_.resetSetting();
                    break;
                }
                case kControlP: {
                    control_panel_.resetSetting();
                    break;
                }
                case kOtherP: {
                    other_panel_.resetSetting();
                    break;
                }
            }
        };
        close_button_.getButton().onClick = [this]() {
            setVisible(false);
        };

        panel_name_laf_.setFontScale(1.5f);
        panel_labels_[0].setText("Colour", juce::dontSendNotification);
        panel_labels_[1].setText("Control", juce::dontSendNotification);
        panel_labels_[2].setText("Other", juce::dontSendNotification);
        for (auto &pL: panel_labels_) {
            pL.setInterceptsMouseClicks(true, false);
            pL.addMouseListener(this, false);
            pL.setJustificationType(juce::Justification::centred);
            pL.setLookAndFeel(&panel_name_laf_);
            addAndMakeVisible(pL);
        }

        label_laf_.setFontScale(1.125f);
        label_laf_.setAlpha(.5f);
        version_label_.setText(
            juce::String(ZLEQUALIZER_CURRENT_VERSION) + " " + juce::String(ZLEQUALIZER_CURRENT_HASH),
            juce::dontSendNotification);
        version_label_.setJustificationType(juce::Justification::bottomLeft);
        version_label_.setLookAndFeel(&label_laf_);
        addAndMakeVisible(version_label_);
    }

    UISettingPanel::~UISettingPanel() {
        for (auto &pL: panel_labels_) {
            pL.setLookAndFeel(nullptr);
        }
        version_label_.setLookAndFeel(nullptr);
    }

    void UISettingPanel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getBackgroundColor());
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight() * 1.25f);
        ui_base_.fillRoundedShadowRectangle(g, bound, 0.5f * ui_base_.getFontSize(), {.blur_radius = 0.5f});
    }

    void UISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight());
        {
            auto label_bound = bound.removeFromTop(ui_base_.getFontSize() * 3.f);
            const auto label_width = label_bound.getWidth() / static_cast<float>(panel_labels_.size());
            for (auto & panel_label : panel_labels_) {
                panel_label.setBounds(label_bound.removeFromLeft(label_width).toNearestInt());
            }
        }

        colour_panel_.setBounds(0, 0,
                              juce::roundToInt(bound.getWidth()),
                              juce::roundToInt(ui_base_.getFontSize() * (ColourSettingPanel::kHeightP + 1.f)));
        control_panel_.setBounds(0, 0,
                               juce::roundToInt(bound.getWidth()),
                               juce::roundToInt(ui_base_.getFontSize() * (ControlSettingPanel::kHeightP + 1.f)));
        other_panel_.setBounds(0, 0,
                             juce::roundToInt(bound.getWidth()),
                             juce::roundToInt(ui_base_.getFontSize() * (OtherUISettingPanel::kHeightP + 1.f)));

        view_port_.setBounds(bound.removeFromTop(bound.getHeight() * .9125f).toNearestInt());
        const auto left_bound = bound.removeFromLeft(
            bound.getWidth() * .3333333f).withSizeKeepingCentre(
            ui_base_.getFontSize() * 2.f, ui_base_.getFontSize() * 2.f);
        const auto center_bound = bound.removeFromLeft(
            bound.getWidth() * .5f).withSizeKeepingCentre(
            ui_base_.getFontSize() * 2.f, ui_base_.getFontSize() * 2.f);
        const auto right_bound = bound.withSizeKeepingCentre(
            ui_base_.getFontSize() * 2.f, ui_base_.getFontSize() * 2.f);
        save_button_.setBounds(left_bound.toNearestInt());
        reset_button_.setBounds(center_bound.toNearestInt());
        close_button_.setBounds(right_bound.toNearestInt());

        bound = getLocalBounds().toFloat();
        bound = bound.removeFromBottom(2.f * ui_base_.getFontSize());
        bound = bound.removeFromLeft(bound.getWidth() * .125f);
        bound.removeFromLeft(ui_base_.getFontSize() * .25f);
        bound.removeFromBottom(ui_base_.getFontSize() * .0625f);
        version_label_.setBounds(bound.toNearestInt());
    }

    void UISettingPanel::loadSetting() {
        colour_panel_.loadSetting();
        control_panel_.loadSetting();
        other_panel_.loadSetting();
    }

    void UISettingPanel::mouseDown(const juce::MouseEvent &event) {
        for (size_t i = 0; i < panel_labels_.size(); ++i) {
            if (event.originalComponent == &panel_labels_[i]) {
                current_panel_idx_ = static_cast<PanelIdx>(i);
                changeDisplayPanel();
                return;
            }
        }
    }

    void UISettingPanel::changeDisplayPanel() {
        switch (current_panel_idx_) {
            case kColourP: {
                view_port_.setViewedComponent(&colour_panel_, false);
                break;
            }
            case kControlP: {
                view_port_.setViewedComponent(&control_panel_, false);
                break;
            }
            case kOtherP: {
                view_port_.setViewedComponent(&other_panel_, false);
                break;
            }
        }
    }

    void UISettingPanel::visibilityChanged() {
        ui_base_.setProperty(zlgui::SettingIdx::kUISettingPanelShow, isVisible());
    }

    void UISettingPanel::setRendererList(const juce::StringArray &rendererList) {
        other_panel_.setRendererList(rendererList);
    }
} // zlpanel
