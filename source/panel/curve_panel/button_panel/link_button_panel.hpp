// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef LINK_BUTTON_PANEL_HPP
#define LINK_BUTTON_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../panel_definitons.hpp"

namespace zlPanel {
    class LinkButtonPanel final : public juce::Component,
                                  private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit LinkButtonPanel(size_t idx,
                                 juce::AudioProcessorValueTreeState &parameters,
                                 juce::AudioProcessorValueTreeState &parametersNA,
                                 zlInterface::UIBase &base,
                                 zlInterface::Dragger &sideDragger);

        ~LinkButtonPanel() override;

        zlInterface::CompactButton &getButton() { return dynLinkC; }

        void updateBound();

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlInterface::Dragger &sideDraggerRef;
        zlInterface::CompactButton dynLinkC;
        juce::Rectangle<float> buttonBound;
        std::atomic<bool> buttonChanged{false};
        juce::OwnedArray<zlInterface::ButtonCusAttachment<false>> buttonAttachments;
        const std::unique_ptr<juce::Drawable> linkDrawable;

        std::atomic<size_t> bandIdx;
        std::atomic<bool> isDynamicON{false}, isSelected{false};
        std::atomic<float> sideFreq{1000.f};

        constexpr static std::array IDs{zlDSP::dynamicON::ID};
        constexpr static std::array NAIDs{zlState::selectedBandIdx::ID};

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlPanel

#endif //LINK_BUTTON_PANEL_HPP
