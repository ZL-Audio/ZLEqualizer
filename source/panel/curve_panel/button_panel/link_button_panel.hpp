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

namespace zlpanel {
    class LinkButtonPanel final : public juce::Component,
                                  private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit LinkButtonPanel(size_t idx,
                                 juce::AudioProcessorValueTreeState &parameters,
                                 juce::AudioProcessorValueTreeState &parametersNA,
                                 zlgui::UIBase &base,
                                 zlgui::Dragger &sideDragger);

        ~LinkButtonPanel() override;

        zlgui::CompactButton &getButton() { return dynLinkC; }

        void updateBound();

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlgui::UIBase &uiBase;
        zlgui::Dragger &sideDraggerRef;
        zlgui::CompactButton dynLinkC;
        float buttonSize{}, buttonBottom{};
        std::atomic<bool> buttonChanged{false};
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > buttonAttachments;
        const std::unique_ptr<juce::Drawable> linkDrawable;

        std::atomic<size_t> bandIdx;
        std::atomic<bool> isDynamicON{false}, isSelected{false};

        constexpr static std::array IDs{zlp::dynamicON::ID};
        constexpr static std::array NAIDs{zlstate::selectedBandIdx::ID};

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlpanel
