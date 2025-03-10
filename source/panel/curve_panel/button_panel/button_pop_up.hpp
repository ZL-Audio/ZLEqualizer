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
#include "button_pop_up_background.hpp"

namespace zlPanel {
    class ButtonPopUp final : public juce::Component,
                              public juce::ComponentListener {
    public:
        explicit ButtonPopUp(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPopUp() override;

        void updateBounds() {
            if (toUpdateBounds == true) {
                toUpdateBounds = false;
                handleAsyncUpdate();
                setBounds(popUpBound.toNearestInt());
            }
        }

        void resized() override;

        void componentMovedOrResized(Component &component, bool wasMoved, bool wasResized) override;

        void setFType(const zlFilter::FilterType f) {
            fType.store(f);
        }

    private:
        size_t band;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;

        juce::RangedAudioParameter *freqPara;

        static constexpr float width{7.7916666f}, height{4.16667f};
        std::atomic<zlFilter::FilterType> fType;

        ButtonPopUpBackground background;

        juce::Label pitchLabel;
        zlInterface::NameLookAndFeel pitchLAF;

        static constexpr std::array pitchLookUp{
            "A", "A#", "B", "C",
            "C#", "D", "D#", "E",
            "F", "F#", "G", "G#"
        };

        void handleAsyncUpdate();

        bool toUpdateBounds{false};
        juce::Rectangle<float> popUpBound;
    };
} // zlPanel
