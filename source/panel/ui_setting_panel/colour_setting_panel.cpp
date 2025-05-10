// Copyright (C) 2025 - zsliu98
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

    ColourSettingPanel::ColourSettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : pRef(p), ui_base_(base), name_laf_(base),
          text_selector_(base, *this, false),
          background_selector_(base, *this, false),
          shadow_selector_(base, *this, false),
          glow_selector_(base, *this, false),
          pre_selector_(base, *this),
          post_selector_(base, *this),
          side_selector_(base, *this),
          grid_selector_(base, *this),
          tag_selector_(base, *this),
          gain_selector_(base, *this),
          side_loudness_selector_(base, *this),
          c_map1_selector_(base), c_map2_selector_(base) {
        juce::ignoreUnused(pRef);
        if (!kSettingDirectory.isDirectory()) {
            kSettingDirectory.createDirectory();
        }
        name_laf_.setFontScale(zlgui::kFontHuge);
        for (size_t i = 0; i < kNumSelectors; ++i) {
            selector_labels_[i].setText(kSelectorNames[i], juce::dontSendNotification);
            selector_labels_[i].setJustificationType(juce::Justification::centredRight);
            selector_labels_[i].setLookAndFeel(&name_laf_);
            addAndMakeVisible(selector_labels_[i]);
            addAndMakeVisible(selectors_[i]);
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
            selectors_[i]->setColour(ui_base_.getColourByIdx(kColourIdx[i]));
        }
        c_map1_selector_.getBox().setSelectedId(static_cast<int>(ui_base_.getCMap1Idx()) + 1);
        c_map2_selector_.getBox().setSelectedId(static_cast<int>(ui_base_.getCMap2Idx()) + 1);
    }

    void ColourSettingPanel::saveSetting() {
        for (size_t i = 0; i < kNumSelectors; ++i) {
            ui_base_.setColourByIdx(kColourIdx[i], selectors_[i]->getColour());
        }
        ui_base_.setCMap1Idx(static_cast<size_t>(c_map1_selector_.getBox().getSelectedId() - 1));
        ui_base_.setCMap2Idx(static_cast<size_t>(c_map2_selector_.getBox().getSelectedId() - 1));
        ui_base_.saveToAPVTS();
    }

    void ColourSettingPanel::resetSetting() {
        text_selector_.setColour(getIntColour(247, 246, 244, 1.f));
        background_selector_.setColour(getIntColour((255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, 1.f));
        shadow_selector_.setColour(getIntColour(0, 0, 0, 1.f));
        glow_selector_.setColour(getIntColour(70, 66, 62, 1.f));
        pre_selector_.setColour(getIntColour(255 - 8, 255 - 9, 255 - 11, .1f));
        post_selector_.setColour(getIntColour(255 - 8, 255 - 9, 255 - 11, .1f));
        side_selector_.setColour(getIntColour(252, 18, 197, .1f));
        grid_selector_.setColour(getIntColour(255 - 8, 255 - 9, 255 - 11, .25f));
        c_map1_selector_.getBox().setSelectedId(zlstate::colourMap1Idx::defaultI + 1);
        c_map2_selector_.getBox().setSelectedId(zlstate::colourMap2Idx::defaultI + 1);
        saveSetting();
    }

    void ColourSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        for (size_t i = 0; i < kNumSelectors; ++i) {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            selector_labels_[i].setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            selectors_[i]->setBounds(local_bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            c_map1_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            c_map1_selector_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            c_map2_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            c_map2_selector_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            import_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .45f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .10f);
            export_label_.setBounds(local_bound.toNearestInt());
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
                    for (size_t i = 0; i < kTagNames.size(); ++i) {
                        if (const auto *xml_colour = xml_input->getChildByName(kTagNames[i])) {
                            const juce::Colour colour = getIntColour(
                                xml_colour->getIntAttribute("r"),
                                xml_colour->getIntAttribute("g"),
                                xml_colour->getIntAttribute("b"),
                                static_cast<float>(xml_colour->getDoubleAttribute("o")));
                            ui_base_.setColourByIdx(kColourIdx[i], colour);
                        }
                    }
                    ui_base_.saveToAPVTS();
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
                    for (size_t i = 0; i < kTagNames.size(); ++i) {
                        auto *xml_colour = xml_output.createNewChildElement(kTagNames[i]);
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
