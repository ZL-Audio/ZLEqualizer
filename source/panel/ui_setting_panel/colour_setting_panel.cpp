// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

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

    ColourSettingPanel::ColourSettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : pRef(p), base_(base), name_laf_(base),
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
                base, *this, i > 3,
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
        import_label_.setText("Import Colours", juce::dontSendNotification);
        import_label_.setJustificationType(juce::Justification::centred);
        import_label_.setLookAndFeel(&name_laf_);
        import_label_.addMouseListener(this, false);
        addAndMakeVisible(import_label_);
        export_label_.setText("Export Colours", juce::dontSendNotification);
        export_label_.setJustificationType(juce::Justification::centred);
        export_label_.setLookAndFeel(&name_laf_);
        export_label_.addMouseListener(this, false);
        addAndMakeVisible(export_label_);
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

        return padding * 13 + slider_height * 12;
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
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            c_map1_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            c_map1_selector_.setBounds(local_bound.removeFromLeft(slider_width * 4 + padding).reduced(0, padding / 3));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            c_map2_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            c_map2_selector_.setBounds(local_bound.removeFromLeft(slider_width * 4 + padding).reduced(0, padding / 3));
        } {
            bound.removeFromTop(padding);
            const auto label_width = bound.getWidth() / 2;
            auto local_bound = bound.removeFromTop(slider_height);
            import_label_.setBounds(local_bound.removeFromLeft(label_width));
            export_label_.setBounds(local_bound.removeFromRight(label_width));
        }
    }

    void ColourSettingPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent == &import_label_) {
            chooser_ = std::make_unique<juce::FileChooser>(
                "Load the colour settings...", kSettingDirectory, "*.xml",
                true, false, nullptr);
            constexpr auto setting_open_flags = juce::FileBrowserComponent::openMode |
                                                juce::FileBrowserComponent::canSelectFiles;
            chooser_->launchAsync(setting_open_flags, [this](const juce::FileChooser &chooser) {
                if (chooser.getResults().size() <= 0) { return; }
                const juce::File settingFile(chooser.getResult());
                if (const auto xml_input = juce::XmlDocument::parse(settingFile)) {
                    for (size_t i = 0; i < 4; ++i) {
                        if (const auto *xml_colour = xml_input->getChildByName(std::string(zlgui::kColourNames[i]))) {
                            const juce::Colour colour = getIntColour(
                                xml_colour->getIntAttribute("r"),
                                xml_colour->getIntAttribute("g"),
                                xml_colour->getIntAttribute("b"),
                                static_cast<float>(xml_colour->getDoubleAttribute("o")));
                            base_.setColourByIdx(static_cast<zlgui::ColourIdx>(i), colour);
                        }
                    }
                    base_.saveToAPVTS();
                    loadSetting();
                }
            });
        } else if (event.originalComponent == &export_label_) {
            chooser_ = std::make_unique<juce::FileChooser>(
                "Save the colour settings...", kSettingDirectory.getChildFile("colour.xml"), "*.xml",
                true, false, nullptr);
            constexpr auto setting_save_flags = juce::FileBrowserComponent::saveMode |
                                                juce::FileBrowserComponent::warnAboutOverwriting;
            chooser_->launchAsync(setting_save_flags, [this](const juce::FileChooser &chooser) {
                if (chooser.getResults().size() <= 0) { return; }
                juce::File setting_file(chooser.getResult().withFileExtension("xml"));
                if (setting_file.create()) {
                    juce::XmlElement xml_output{"colour_setting"};
                    for (size_t i = 0; i < 4; ++i) {
                        auto *xml_colour = xml_output.createNewChildElement(std::string(zlgui::kColourNames[i]));
                        juce::Colour colour = selectors_[i]->getColour();
                        xml_colour->setAttribute("r", colour.getRed());
                        xml_colour->setAttribute("g", colour.getGreen());
                        xml_colour->setAttribute("b", colour.getBlue());
                        xml_colour->setAttribute("o", colour.getFloatAlpha());
                    }
                    const auto result = xml_output.writeTo(setting_file);
                    juce::ignoreUnused(result);
                }
            });
        }
    }
} // zlpanel
