// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../panel_definitons.hpp"

namespace zlPanel {

class ButtonPopUpBackground: public juce::Component {
public:
        explicit ButtonPopUpBackground(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPopUpBackground() override = default;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        size_t band;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;

        std::atomic<float> width{7.7916666f}, height{4.16667f};
        std::atomic<zlFilter::FilterType> fType;

        zlInterface::CompactButton bypassC, soloC;
        juce::OwnedArray<zlInterface::ButtonCusAttachment<false> > buttonAttachments;
        const std::unique_ptr<juce::Drawable> bypassDrawable, soloDrawable;

        zlInterface::CompactCombobox fTypeC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        const std::unique_ptr<juce::Drawable> drawable;
        zlInterface::ClickButton button;

        static constexpr std::array pitchLookUp{
            "A", "A#", "B", "C",
            "C#", "D", "D#", "E",
            "F", "F#", "G", "G#"
        };

        std::atomic<float> freq{1000.f};

        std::atomic<bool> toUpdateBounds{false};
        juce::Rectangle<float> popUpBound;
};

} // zlPanel
