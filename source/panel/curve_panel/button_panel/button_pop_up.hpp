// Copyright (C) 2024 - zsliu98
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
                              public juce::ComponentListener,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
    public:
        explicit ButtonPopUp(size_t bandIdx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPopUp() override;

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
        std::atomic<float> width{7.7916666f}, height{4.16667f};
        std::atomic<zlIIR::FilterType> fType;

        zlInterface::CompactButton bypassC, soloC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
        const std::unique_ptr<juce::Drawable> bypassDrawable, soloDrawable;

        zlInterface::CompactCombobox fTypeC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        juce::Label pitchLabel;
        zlInterface::NameLookAndFeel pitchLAF;

        const std::unique_ptr<juce::Drawable> drawable;
        zlInterface::ClickButton button;

        static constexpr std::array pitchLookUp{
            "A", "A#", "B", "C",
            "C#", "D", "D#", "E",
            "F", "F#", "G", "G#"
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        std::atomic<float> freq{1000.f};
        void handleAsyncUpdate() override;
    };
} // zlPanel

#endif //ZLEqualizer_BUTTON_POP_UP_HPP
