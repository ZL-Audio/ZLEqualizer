// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlpanel {
    class SettingPanel final : public juce::Component,
                               private juce::MultiTimer, private juce::ValueTree::Listener {
    public:
        explicit SettingPanel(PluginProcessor &p, zlgui::UIBase &base,
                              const juce::String &label, zlgui::BoxIdx idx);

        ~SettingPanel() override;

        void paint(juce::Graphics &g) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        juce::String setting_name_;
        zlgui::BoxIdx box_idx_;

        void timerCallback(int timerID) override;

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override;
    };
} // zlpanel
