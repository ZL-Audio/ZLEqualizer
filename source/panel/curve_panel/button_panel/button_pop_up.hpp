// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_BUTTON_POP_UP_HPP
#define ZLEqualizer_BUTTON_POP_UP_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../panel_definitons.hpp"

namespace zlPanel {
    class ButtonPopUp final : public juce::Component,
                              public juce::ComponentListener {
    public:
        explicit ButtonPopUp(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPopUp() override = default;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void componentMovedOrResized(Component &component, bool wasMoved, bool wasResized) override;

        void setFType(const zlIIR::FilterType f) {
            fType.store(f);
        }

    private:
        std::atomic<size_t> band;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<float> width {5.f}, height {2.5f};
        std::atomic<zlIIR::FilterType> fType;

        zlInterface::CompactButton bypassC, soloC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
        const std::unique_ptr<juce::Drawable> bypassDrawable, soloDrawable;
    };
} // zlPanel

#endif //ZLEqualizer_BUTTON_POP_UP_HPP
