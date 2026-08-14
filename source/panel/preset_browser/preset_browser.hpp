// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"

#include "group_list.hpp"
#include "preset_entry.hpp"
#include "preset_json.hpp"
#include "preset_list.hpp"
#include "rounded_text_editor.hpp"
#include "warning_overlay.hpp"
#include "../../gui/popup/panel_surface_background.hpp"

namespace zlpanel {
    class PresetBrowser final : public juce::Component,
                                private juce::ValueTree::Listener {
    public:
        explicit PresetBrowser(PluginProcessor& processor, zlgui::UIBase& base);

        ~PresetBrowser() override;

        void resized() override;

        void lookAndFeelChanged() override;

        void visibilityChanged() override;

        void refresh();

        void flushPendingScroll();

        int getIdealWidth() const;

        int getIdealHeight() const;

    private:
        PluginProcessor& processor_;
        zlgui::UIBase& base_;
        const juce::File presets_directory_;

        zlgui::popup::PanelSurfaceBackground background_;

        std::unique_ptr<juce::Drawable> delete_drawable_;
        std::unique_ptr<juce::Drawable> close_drawable_;
        std::unique_ptr<juce::Drawable> folder_open_drawable_;
        zlgui::button::ClickButton delete_group_button_;
        zlgui::button::ClickButton delete_preset_button_;
        zlgui::button::ClickButton close_button_;
        zlgui::button::ClickButton folder_open_button_;

        juce::Label group_label_;
        juce::Label preset_label_;
        RoundedTextEditor search_editor_;
        RoundedTextEditor group_name_editor_;
        RoundedTextEditor preset_name_editor_;

        GroupList group_list_;
        PresetList preset_list_;
        WarningOverlay warning_overlay_;

        juce::StringArray groups_;
        std::vector<PresetEntry> all_presets_;
        std::vector<PresetEntry> presets_;
        juce::String selected_group_{"All Presets"};
        juce::File selected_preset_file_;

        static constexpr auto kAllPresetsGroup = "All Presets";
        static constexpr auto kDefaultGroup = "User";
        static constexpr auto kPresetExtension = ".json";

        bool ensurePresetDirectory();

        void refreshGroups();

        void refreshPresetCache();

        void refreshPresets();

        void createGroup();

        void deleteSelectedGroup();

        void savePreset();

        void writePreset(const juce::File& file);

        void loadPreset(const juce::File& file);

        void deleteSelectedPreset();

        void selectGroup(const juce::String& group);

        void selectPreset(const juce::File& file, bool scroll_to_row = false);

        void configureEditor(RoundedTextEditor& editor, const juce::String& placeholder);

        void applyColours();

        void showError(const juce::String& message);

        void updateDeleteButtonStates();

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;

        const PresetEntry* getSelectedPreset() const;

        juce::String getWritableGroup() const;

        void revealFolder();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
    };
}
