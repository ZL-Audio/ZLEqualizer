// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "preset_browser.hpp"

#include <algorithm>

#include "BinaryData.h"
#include "preset_list_layout.hpp"

namespace zlpanel {
    namespace {
        juce::File getPresetsDirectory() {
            return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("ZL Audio")
                .getChildFile(JucePlugin_Name)
                .getChildFile("Presets");
        }

        juce::File getLegacyPresetsDirectory() {
            return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile(JucePlugin_Name)
                .getChildFile("Presets");
        }

        juce::Result ensureDirectoryExists(const juce::File& directory) {
            if (directory.isDirectory()) {
                return juce::Result::ok();
            }

            const auto parent = directory.getParentDirectory();
            if (parent == directory) {
                return juce::Result::fail("Could not find a parent directory");
            }
            if (const auto result = ensureDirectoryExists(parent); result.failed()) {
                return result;
            }

            const auto result = directory.createDirectory();
            return result.wasOk() || directory.isDirectory() ? juce::Result::ok() : result;
        }

        juce::Result migrateLegacyPresets(const juce::File& destination) {
            const auto legacy = getLegacyPresetsDirectory();
            if (!legacy.isDirectory() || destination.exists()) {
                return juce::Result::ok();
            }

            const auto parent = destination.getParentDirectory();
            if (const auto result = ensureDirectoryExists(parent); result.failed()) {
                return result;
            }

            const auto temporary = parent.getNonexistentChildFile(".Preset Migration", {}, false);
            if (!legacy.copyDirectoryTo(temporary)) {
                [[maybe_unused]] const auto flag = temporary.deleteRecursively();
                return juce::Result::fail("Could not copy the existing presets");
            }
            if (!temporary.moveFileTo(destination)) {
                [[maybe_unused]] const auto flag = temporary.deleteRecursively();
                return destination.isDirectory()
                    ? juce::Result::ok()
                    : juce::Result::fail("Could not finish moving the existing presets");
            }
            return juce::Result::ok();
        }

        juce::String getDirectoryCreationError(const juce::String& message,
                                               const juce::File& directory,
                                               const juce::Result& result) {
            return message + "\nReason: " + result.getErrorMessage() +
                "\nPath: " + directory.getFullPathName();
        }

        bool isVisibleFileName(const juce::String& name) {
            return name.isNotEmpty() && !name.startsWithChar('.');
        }
    }

    PresetBrowser::PresetBrowser(PluginProcessor& processor, zlgui::UIBase& base) :
        processor_(processor),
        base_(base),
        presets_directory_(getPresetsDirectory()),
        background_(base),
        delete_drawable_(juce::Drawable::createFromImageData(BinaryData::trash_svg,
                                                             BinaryData::trash_svgSize)),
        close_drawable_(juce::Drawable::createFromImageData(BinaryData::close_svg,
                                                            BinaryData::close_svgSize)),
        folder_open_drawable_(juce::Drawable::createFromImageData(BinaryData::folder_open_svg,
                                                                  BinaryData::folder_open_svgSize)),
        delete_group_button_(base, delete_drawable_.get(), nullptr, ""),
        delete_preset_button_(base, delete_drawable_.get(), nullptr, ""),
        close_button_(base, close_drawable_.get(), nullptr, ""),
        folder_open_button_(base, folder_open_drawable_.get(), nullptr, ""),
        group_label_({}, "Groups"),
        preset_label_({}, "Presets"),
        search_editor_(base),
        group_name_editor_(base),
        preset_name_editor_(base),
        group_list_(base),
        preset_list_(base),
        warning_overlay_(base) {
        setOpaque(false);
        setInterceptsMouseClicks(true, true);

        background_.setBufferedToImage(true);
        addAndMakeVisible(background_);

        for (auto* label : {&group_label_, &preset_label_}) {
            label->setJustificationType(juce::Justification::centredLeft);
            label->setBufferedToImage(true);
            addAndMakeVisible(label);
        }
        configureEditor(search_editor_, "Search Presets");
        configureEditor(group_name_editor_, "New Group");
        configureEditor(preset_name_editor_, "New Preset (Enter to Save)");
        search_editor_.onTextChange = [this]() { refreshPresets(); };
        group_name_editor_.onReturnKey = [this]() {
            createGroup();
            group_name_editor_.giveAwayKeyboardFocus();
        };
        preset_name_editor_.onReturnKey = [this]() {
            savePreset();
            preset_name_editor_.giveAwayKeyboardFocus();
        };

        group_list_.onGroupSelected = [this](const auto& group) { selectGroup(group); };
        preset_list_.onPresetSelected = [this](const auto& file) { selectPreset(file); };
        preset_list_.onPresetLoad = [this](const auto& file) { loadPreset(file); };
        addAndMakeVisible(group_list_);
        addAndMakeVisible(preset_list_);

        for (auto* button : {&delete_group_button_, &delete_preset_button_, &close_button_, &folder_open_button_}) {
            button->setImageAlpha(.55f, 1.f);
            button->setBufferedToImage(true);
            addAndMakeVisible(button);
        }
        delete_group_button_.getButton().onClick = [this]() {
            deleteSelectedGroup();
        };
        delete_preset_button_.getButton().onClick = [this]() {
            deleteSelectedPreset();
        };
        close_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::PanelSettingIdx::kPresetBrowser, 0.f);
        };
        folder_open_button_.getButton().onClick = [this]() {
            revealFolder();
        };

        addChildComponent(warning_overlay_);
        warning_overlay_.setBufferedToImage(true);

        base_.getPanelValueTree().addListener(this);
        base_.setPanelProperty(zlgui::PanelSettingIdx::kPresetBrowser, 0.f);

        applyColours();

        setWantsKeyboardFocus(true);
    }

    PresetBrowser::~PresetBrowser() {
        base_.getPanelValueTree().removeListener(this);
    }

    void PresetBrowser::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = juce::roundToInt(font_size * .72f);
        const auto button_size = juce::roundToInt(font_size * 2.f);
        const auto row_height = juce::roundToInt(font_size * 1.9f);
        const auto header_height = button_size;
        const auto footer_height = button_size + padding;
        const auto list_top_margin = juce::roundToInt(font_size * .36f);
        const auto column_text_inset = preset_list_layout::columnTextInset(font_size);

        auto bounds = getLocalBounds().reduced(padding);
        auto header = bounds.removeFromTop(header_height);
        auto footer = bounds.removeFromBottom(footer_height);
        footer.removeFromTop(padding);

        const auto minimum_group_width = juce::roundToInt(font_size * 10.8f);
        const auto group_width = juce::jlimit(minimum_group_width,
                                              juce::jmax(minimum_group_width, bounds.getWidth() / 2),
                                              juce::roundToInt(static_cast<float>(bounds.getWidth()) * .27f));

        auto group_header = header.removeFromLeft(group_width);
        header.removeFromLeft(padding);
        delete_group_button_.setBounds(group_header.removeFromRight(button_size));
        group_header.removeFromRight(padding / 2);
        folder_open_button_.setBounds(group_header.removeFromRight(button_size));
        folder_open_button_.getButton().setEdgeIndent(static_cast<int>(std::round(font_size * .175f)));
        group_label_.setBounds(group_header);

        close_button_.setBounds(header.removeFromRight(button_size));
        header.removeFromRight(padding / 2);
        delete_preset_button_.setBounds(header.removeFromRight(button_size));
        header.removeFromRight(padding / 2);
        const auto preset_label_width = juce::jmin(header.getWidth() / 3,
                                                   juce::roundToInt(font_size * 7.5f));
        preset_label_.setBounds(header.removeFromLeft(preset_label_width));
        search_editor_.setBounds(header);

        bounds.removeFromTop(list_top_margin);
        auto group_area = bounds.removeFromLeft(group_width);
        bounds.removeFromLeft(padding);
        group_list_.setBounds(group_area);
        preset_list_.setBounds(bounds);

        auto group_footer = footer.removeFromLeft(group_width);
        footer.removeFromLeft(padding);
        group_name_editor_.setBounds(group_footer);
        preset_name_editor_.setBounds(footer);

        background_.setBounds(getLocalBounds());
        background_.setSurfaceBounds({
            search_editor_.getBounds(),
            group_list_.getBounds(),
            preset_list_.getBounds(),
            group_name_editor_.getBounds(),
            preset_name_editor_.getBounds()
        });

        const auto editor_top_inset = juce::roundToInt(font_size * .16f);
        search_editor_.setIndents(column_text_inset, editor_top_inset);
        group_name_editor_.setIndents(column_text_inset, editor_top_inset);
        preset_name_editor_.setIndents(column_text_inset, editor_top_inset);
        group_label_.setBorderSize({0, column_text_inset, 0, 0});
        preset_label_.setBorderSize({0, column_text_inset, 0, 0});

        const juce::FontOptions editor_font{1.5f * font_size};
        for (auto* editor : {&search_editor_, &group_name_editor_, &preset_name_editor_}) {
            editor->setFont(editor_font);
            editor->applyFontToAllText(editor_font);
        }
        const juce::FontOptions heading_font{1.5f * font_size};
        group_label_.setFont(heading_font);
        preset_label_.setFont(heading_font);
        group_list_.setRowHeight(row_height);
        preset_list_.setRowHeight(row_height);

        warning_overlay_.setBounds(getLocalBounds());
    }

    void PresetBrowser::lookAndFeelChanged() {
        applyColours();
        background_.repaint();
        repaint();
    }

    void PresetBrowser::visibilityChanged() {
        if (isVisible()) {
            refresh();
        } else {
            warning_overlay_.hide();
        }
    }

    void PresetBrowser::refresh() {
        if (!ensurePresetDirectory()) {
            return;
        }
        refreshGroups();
        refreshPresetCache();
        refreshPresets();
    }

    void PresetBrowser::flushPendingScroll() {
        group_list_.flushPendingScroll();
        preset_list_.flushPendingScroll();
    }

    int PresetBrowser::getIdealWidth() const {
        return juce::roundToInt(base_.getFontSize() * 48.f);
    }

    int PresetBrowser::getIdealHeight() const {
        return juce::roundToInt(base_.getFontSize() * 31.f);
    }

    bool PresetBrowser::ensurePresetDirectory() {
        if (const auto result = migrateLegacyPresets(presets_directory_); result.failed()) {
            showError(getDirectoryCreationError("Could not migrate the existing presets.",
                                                getLegacyPresetsDirectory(), result));
        }

        if (const auto result = ensureDirectoryExists(presets_directory_); result.failed()) {
            showError(getDirectoryCreationError("Could not create the preset directory.",
                                                presets_directory_, result));
            return false;
        }

        const auto user_directory = presets_directory_.getChildFile(kDefaultGroup);
        if (const auto result = ensureDirectoryExists(user_directory); result.failed()) {
            showError(getDirectoryCreationError("Could not create the User group.",
                                                user_directory, result));
            return false;
        }
        return true;
    }

    void PresetBrowser::refreshGroups() {
        auto directories = presets_directory_.findChildFiles(juce::File::findDirectories |
                                                             juce::File::ignoreHiddenFiles,
                                                             false);
        std::sort(directories.begin(), directories.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs == rhs) {
                return false;
            }
            if (lhs.getFileName() == kDefaultGroup) {
                return true;
            }
            if (rhs.getFileName() == kDefaultGroup) {
                return false;
            }
            return lhs.getFileName().compareNatural(rhs.getFileName()) < 0;
        });

        groups_.clear();
        groups_.add(kAllPresetsGroup);
        for (const auto& directory : directories) {
            groups_.add(directory.getFileName());
        }
        if (!groups_.contains(selected_group_)) {
            selected_group_ = kDefaultGroup;
        }
        group_list_.setGroups(groups_, selected_group_);
        updateDeleteButtonStates();
    }

    void PresetBrowser::refreshPresetCache() {
        const auto files = presets_directory_.findChildFiles(juce::File::findFiles |
                                                             juce::File::ignoreHiddenFiles,
                                                             true, "*" + juce::String{kPresetExtension});
        all_presets_.clear();
        all_presets_.reserve(static_cast<size_t>(files.size()));
        for (const auto& file : files) {
            all_presets_.push_back({file.getFileNameWithoutExtension(),
                                    file.getParentDirectory().getFileName(), file});
        }
        std::sort(all_presets_.begin(), all_presets_.end(), [](const auto& lhs, const auto& rhs) {
            const auto group_order = lhs.group.compareNatural(rhs.group);
            return group_order == 0 ? lhs.name.compareNatural(rhs.name) < 0 : group_order < 0;
        });
    }

    void PresetBrowser::refreshPresets() {
        const auto search = search_editor_.getText().trim().toLowerCase();
        presets_.clear();
        presets_.reserve(all_presets_.size());
        for (const auto& preset : all_presets_) {
            if (selected_group_ != kAllPresetsGroup && preset.group != selected_group_) {
                continue;
            }
            if (search.isNotEmpty() && !preset.name.toLowerCase().contains(search) &&
                !preset.group.toLowerCase().contains(search)) {
                continue;
            }
            presets_.push_back(preset);
        }

        if (getSelectedPreset() == nullptr) {
            selected_preset_file_ = juce::File{};
        }
        preset_list_.setPresets(presets_, selected_preset_file_, selected_group_ == kAllPresetsGroup);
        updateDeleteButtonStates();
    }

    void PresetBrowser::createGroup() {
        const auto requested_name = group_name_editor_.getText().trim();
        const auto legal_name = juce::File::createLegalFileName(requested_name);
        if (!isVisibleFileName(legal_name) || legal_name.equalsIgnoreCase(kAllPresetsGroup)) {
            showError("Enter a valid group name.");
            return;
        }

        const auto directory = presets_directory_.getChildFile(legal_name);
        if (const auto result = ensureDirectoryExists(directory); result.failed()) {
            showError(getDirectoryCreationError("Could not create group \"" + legal_name + "\".",
                                                directory, result));
            return;
        }

        selected_group_ = legal_name;
        group_name_editor_.clear();
        refresh();
    }

    void PresetBrowser::deleteSelectedGroup() {
        if (selected_group_ == kAllPresetsGroup || selected_group_ == kDefaultGroup) {
            showError("This group cannot be deleted.");
            return;
        }

        const auto directory = presets_directory_.getChildFile(selected_group_);
        if (!directory.isDirectory()) {
            showError("The group is no longer available.");
            refresh();
            return;
        }

        const auto group = selected_group_;
        const auto preset_count = static_cast<int>(std::count_if(all_presets_.begin(), all_presets_.end(),
                                                                 [&group](const auto& preset) {
                                                                     return preset.group == group;
                                                                 }));
        const auto count_text = juce::String{preset_count} + (preset_count == 1 ? " preset" : " presets");
        warning_overlay_.show("Delete \"" + group + "\" and all of its contents (" +
                              count_text + ")?",
                              "Delete", true, [this, directory, group]() {
                                  if (!directory.deleteRecursively()) {
                                      showError("Could not delete group \"" + group + "\".");
                                      return;
                                  }
                                  selected_group_ = kDefaultGroup;
                                  selected_preset_file_ = juce::File{};
                                  preset_name_editor_.clear();
                                  refresh();
                              });
    }

    void PresetBrowser::savePreset() {
        const auto requested_name = preset_name_editor_.getText().trim();
        auto legal_name = juce::File::createLegalFileName(requested_name);
        if (legal_name.endsWithIgnoreCase(kPresetExtension)) {
            legal_name = legal_name.dropLastCharacters(juce::String{kPresetExtension}.length());
        }
        if (!isVisibleFileName(legal_name)) {
            showError("Enter a preset name.");
            return;
        }

        const auto group = getWritableGroup();
        const auto file = presets_directory_.getChildFile(group)
            .getChildFile(legal_name + kPresetExtension);
        if (!file.existsAsFile()) {
            writePreset(file);
            return;
        }

        warning_overlay_.show("A preset named \"" + legal_name + "\" already exists in \"" + group + "\".",
                              "Replace", true, [this, file]() { writePreset(file); });
    }

    void PresetBrowser::writePreset(const juce::File& file) {
        juce::MemoryBlock state;
        processor_.getStateInformation(state);
        if (const auto result = PresetJson::write(file, state); result.failed()) {
            showError("Could not save preset \"" + file.getFileNameWithoutExtension() + "\".");
            return;
        }

        selected_group_ = file.getParentDirectory().getFileName();
        selected_preset_file_ = file;
        search_editor_.setText({}, juce::dontSendNotification);
        preset_name_editor_.clear();
        refresh();
        selectPreset(file, true);
    }

    void PresetBrowser::loadPreset(const juce::File& file) {
        const auto iterator = std::find_if(presets_.begin(), presets_.end(), [&file](const auto& preset) {
            return preset.file == file;
        });
        if (iterator == presets_.end()) {
            showError("The preset is no longer available.");
            return;
        }

        juce::MemoryBlock state;
        if (const auto result = PresetJson::read(file, state); result.failed()) {
            showError("Preset \"" + iterator->name + "\" is invalid.");
            return;
        }

        processor_.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        processor_.updateHostDisplay(juce::AudioProcessorListener::ChangeDetails()
            .withNonParameterStateChanged(true));
        selectPreset(file);
    }

    void PresetBrowser::deleteSelectedPreset() {
        const auto* selected = getSelectedPreset();
        if (selected == nullptr) {
            showError("Select a preset to delete.");
            return;
        }

        const auto preset = *selected;
        warning_overlay_.show("Delete \"" + preset.name + "\" from \"" + preset.group + "\"?",
                              "Delete", true, [this, preset]() {
                                  if (!preset.file.deleteFile()) {
                                      showError("Could not delete \"" + preset.name + "\".");
                                      return;
                                  }
                                  selected_preset_file_ = juce::File{};
                                  preset_name_editor_.clear();
                                  refreshPresetCache();
                                  refreshPresets();
                              });
    }

    void PresetBrowser::selectGroup(const juce::String& group) {
        if (!groups_.contains(group) || selected_group_ == group) {
            return;
        }
        selected_group_ = group;
        selected_preset_file_ = juce::File{};
        preset_name_editor_.clear();
        group_list_.setGroups(groups_, selected_group_);
        refreshPresets();
    }

    void PresetBrowser::selectPreset(const juce::File& file, const bool scroll_to_row) {
        const auto iterator = std::find_if(presets_.begin(), presets_.end(), [&file](const auto& preset) {
            return preset.file == file;
        });
        if (iterator == presets_.end()) {
            selected_preset_file_ = juce::File{};
            preset_list_.selectFile(selected_preset_file_);
            updateDeleteButtonStates();
            return;
        }

        selected_preset_file_ = file;
        preset_list_.selectFile(file, scroll_to_row);
        updateDeleteButtonStates();
    }

    void PresetBrowser::configureEditor(RoundedTextEditor& editor, const juce::String& placeholder) {
        editor.setTextToShowWhenEmpty(placeholder, base_.getTextInactiveColour());
        editor.setJustification(juce::Justification::centredLeft);
        editor.setSelectAllWhenFocused(false);
        addAndMakeVisible(editor);
    }

    void PresetBrowser::applyColours() {
        for (auto* label : {&group_label_, &preset_label_}) {
            label->setColour(juce::Label::textColourId, base_.getTextColour().withAlpha(.82f));
        }
        for (auto* editor : {&search_editor_, &group_name_editor_, &preset_name_editor_}) {
            editor->setColour(juce::TextEditor::textColourId, base_.getTextColour());
            editor->setColour(juce::TextEditor::highlightColourId, base_.getTextColour().withAlpha(.22f));
            editor->setColour(juce::TextEditor::highlightedTextColourId, base_.getTextColour());
            editor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
            editor->setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            editor->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        }
    }

    void PresetBrowser::showError(const juce::String& message) {
        warning_overlay_.showMessage(message);
    }

    void PresetBrowser::updateDeleteButtonStates() {
        const auto can_delete_group = selected_group_ != kAllPresetsGroup && selected_group_ != kDefaultGroup;
        delete_group_button_.setAlpha(can_delete_group ? 1.f : .25f);
        delete_group_button_.setInterceptsMouseClicks(can_delete_group, can_delete_group);

        const auto can_delete_preset = getSelectedPreset() != nullptr;
        delete_preset_button_.setAlpha(can_delete_preset ? 1.f : .25f);
        delete_preset_button_.setInterceptsMouseClicks(can_delete_preset, can_delete_preset);
    }

    void PresetBrowser::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (!base_.isPanelIdentifier(zlgui::PanelSettingIdx::kPresetBrowser, property)) {
            return;
        }

        const auto should_be_visible = static_cast<float>(
            base_.getPanelProperty(zlgui::PanelSettingIdx::kPresetBrowser)) > .5f;
        setVisible(should_be_visible);
        if (should_be_visible) {
            toFront(false);
        }
    }

    const PresetEntry* PresetBrowser::getSelectedPreset() const {
        const auto iterator = std::find_if(presets_.begin(), presets_.end(), [this](const auto& preset) {
            return preset.file == selected_preset_file_;
        });
        return iterator == presets_.end() ? nullptr : &*iterator;
    }

    juce::String PresetBrowser::getWritableGroup() const {
        if (selected_group_ != kAllPresetsGroup) {
            return selected_group_;
        }
        if (const auto* selected = getSelectedPreset(); selected != nullptr) {
            return selected->group;
        }
        return kDefaultGroup;
    }

    void PresetBrowser::revealFolder() {
        if (presets_directory_.isDirectory()) {
            presets_directory_.revealToUser();
        } else {
            showError("Could not open the preset directory.");
        }
    }
}
