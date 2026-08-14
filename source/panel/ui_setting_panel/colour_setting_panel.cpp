// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_setting_panel.hpp"

namespace zlpanel {
    static juce::Colour getIntColour(const int r, const int g, const int b, float alpha) {
        return {
            static_cast<juce::uint8>(r),
            static_cast<juce::uint8>(g),
            static_cast<juce::uint8>(b),
            alpha
        };
    }

    ColourSettingPanel::ColourSettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        pRef(p), base_(base), name_laf_(base),
        c_map1_selector_(base), c_map2_selector_(base) {
        juce::ignoreUnused(pRef);
        if (!kSettingDirectory.isDirectory()) {
            const auto result = kSettingDirectory.createDirectory();
            juce::ignoreUnused(result);
        }
        name_laf_.setFontScale(zlgui::kFontHuge);
        for (size_t i = 0; i < kNumSelectors; ++i) {
            auto label = std::string(zlgui::kColourNames[i]);
            label[0] = static_cast<char>(std::toupper(label[0]));
            label += " Colour";
            selector_labels_[i].setText(label, juce::dontSendNotification);
            selector_labels_[i].setJustificationType(juce::Justification::centredRight);
            selector_labels_[i].setLookAndFeel(&name_laf_);
            addAndMakeVisible(selector_labels_[i]);

            selectors_[i] = std::make_unique<zlgui::colour_selector::ColourOpacitySelector>(
                base, *this, i > 1,
                12.f, 10.f, kSliderWidthScale, kSliderWidthScale);
            addAndMakeVisible(*selectors_[i]);
        }
        c_map1_label_.setText("Colour Map 1", juce::dontSendNotification);
        c_map1_label_.setJustificationType(juce::Justification::centredRight);
        c_map1_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(c_map1_label_);
        addAndMakeVisible(c_map1_selector_);
        c_map2_label_.setText("Colour Map 2", juce::dontSendNotification);
        c_map2_label_.setJustificationType(juce::Justification::centredRight);
        c_map2_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(c_map2_label_);
        addAndMakeVisible(c_map2_selector_);
    }

    ColourSettingPanel::~ColourSettingPanel() {
        for (size_t i = 0; i < kNumSelectors; ++i) {
            selector_labels_[i].setLookAndFeel(nullptr);
        }
    }

    void ColourSettingPanel::loadSetting() {
        for (size_t i = 0; i < kNumSelectors; ++i) {
            selectors_[i]->setColour(base_.getColourByIdx(static_cast<zlgui::ColourIdx>(i)));
        }
        c_map1_selector_.getBox().setSelectedId(static_cast<int>(base_.getCMap1Idx()) + 1);
        c_map2_selector_.getBox().setSelectedId(static_cast<int>(base_.getCMap2Idx()) + 1);
    }

    void ColourSettingPanel::saveSetting() {
        for (size_t i = 0; i < kNumSelectors; ++i) {
            base_.setColourByIdx(static_cast<zlgui::ColourIdx>(i), selectors_[i]->getColour());
        }
        base_.setCMap1Idx(static_cast<size_t>(c_map1_selector_.getBox().getSelectedId() - 1));
        base_.setCMap2Idx(static_cast<size_t>(c_map2_selector_.getBox().getSelectedId() - 1));
        base_.saveToAPVTS();
    }

    void ColourSettingPanel::resetSetting() {
        for (size_t i = 0; i < kNumSelectors; ++i) {
            const auto dv = zlstate::kColourDefaults[i];
            selectors_[i]->setColour(getIntColour(dv.r, dv.g, dv.b, dv.opacity));
        }
        c_map1_selector_.getBox().setSelectedId(zlstate::PColourMap1Idx::kDefaultI + 1);
        c_map2_selector_.getBox().setSelectedId(zlstate::PColourMap2Idx::kDefaultI + 1);
        saveSetting();
    }

    int ColourSettingPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return padding * 12 + slider_height * 11;
    }

    void ColourSettingPanel::resized() {
        auto bound = getLocalBounds();
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderWidthScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        for (size_t i = 0; i < kNumSelectors; ++i) {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            selector_labels_[i].setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            selectors_[i]->setBounds(local_bound);
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            c_map1_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            c_map1_selector_.setBounds(local_bound.removeFromLeft(slider_width * 4 + padding).reduced(0, padding / 3));
        }
        {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            c_map2_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            c_map2_selector_.setBounds(local_bound.removeFromLeft(slider_width * 4 + padding).reduced(0, padding / 3));
        }
    }
}
