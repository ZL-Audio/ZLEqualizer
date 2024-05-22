// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef LINK_BUTTON_PANEL_HPP
#define LINK_BUTTON_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../panel_definitons.hpp"

namespace zlPanel {
    class LinkButtonPanel final : public juce::Component,
    private juce::AudioProcessorValueTreeState::Listener,
    private juce::AsyncUpdater{
    public:
        explicit LinkButtonPanel(size_t idx,
                        juce::AudioProcessorValueTreeState &parameters,
                        juce::AudioProcessorValueTreeState &parametersNA,
                        zlInterface::UIBase &base);

        ~LinkButtonPanel() override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlInterface::CompactButton dynLinkC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
        const std::unique_ptr<juce::Drawable> linkDrawable;

        std::atomic<size_t> bandIdx;
        std::atomic<bool> isDynamicON{false}, isSelected{false};
        std::atomic<float> sideFreq{1000.f};

        constexpr static std::array IDs{zlDSP::sideFreq::ID, zlDSP::dynamicON::ID};
        constexpr static std::array NAIDs{zlState::selectedBandIdx::ID};

        void updateBound();

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
} // zlPanel

#endif //LINK_BUTTON_PANEL_HPP
