// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../state/state.hpp"
#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../panel_definitons.hpp"

namespace zlpanel {
    class OutputValuePanel final : public juce::Component,
                                   private juce::MultiTimer,
                                   private juce::ValueTree::Listener {
    public:
        explicit OutputValuePanel(PluginProcessor &p,
                                  zlgui::UIBase &base);

        ~OutputValuePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        juce::RangedAudioParameter *lm_para_;
        std::atomic<float> &scale_;
        float c_gain_{0.f}, c_scale_{100.f};
        bool c_learning_{false};
        juce::String gain_string_{"0.0"}, scale_string_{"100%"};
        bool show_gain_{false};
        juce::Rectangle<float> gain_bound_, scale_bound_;
        juce::Path background_path_;

        void timerCallback(int timerID) override;

        void lookAndFeelChanged() override;

        void updateGainValue();

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override;
    };
} // zlpanel
