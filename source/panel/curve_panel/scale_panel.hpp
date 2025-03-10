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

namespace zlPanel {
    class ScalePanel final : public juce::Component,
                             public juce::SettableTooltipClient,
                             private juce::AsyncUpdater {
    public:
        ScalePanel(PluginProcessor &processor,
                   zlInterface::UIBase &base);

        ~ScalePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void setMaximumDB(const float x) {
            maximumDB.store(x);
            triggerAsyncUpdate();
        }

        void setMinimumFFTDB(const float x) {
            minimumFFTDB.store(x);
            triggerAsyncUpdate();
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactCombobox scaleBox, minFFTBox;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;
        std::atomic<float> maximumDB{12.f}, minimumFFTDB{-72.f};

        static constexpr std::array<float, 5> scaleDBs = {
            1.f / 6.f, 2.f / 6.f, 0.5, 4.f / 6.f, 5.f / 6.f
        };

        void handleAsyncUpdate() override;
    };
} // zlPanel
