// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../../state/state.hpp"

namespace zlpanel {
    class ScalePanel final : public juce::Component,
                             public juce::SettableTooltipClient,
                             private juce::AsyncUpdater {
    public:
        ScalePanel(PluginProcessor &processor,
                   zlgui::UIBase &base);

        ~ScalePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void setMaximumDB(const float x) {
            maximum_db_.store(x);
            triggerAsyncUpdate();
        }

        void setMinimumFFTDB(const float x) {
            minimum_fft_db_.store(x);
            triggerAsyncUpdate();
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::CompactCombobox scale_box_, min_fft_box_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> box_attachments_;
        std::atomic<float> maximum_db_{12.f}, minimum_fft_db_{-72.f};

        static constexpr std::array<float, 5> kScaleDBs = {
            1.f / 6.f, 2.f / 6.f, 0.5, 4.f / 6.f, 5.f / 6.f
        };

        void handleAsyncUpdate() override;
    };
} // zlpanel
