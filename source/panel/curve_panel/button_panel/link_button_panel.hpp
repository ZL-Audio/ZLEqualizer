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
                                 juce::AudioProcessorValueTreeState &parameters_NA,
                                 zlgui::UIBase &base,
                                 zlgui::Dragger &side_dragger);

        ~LinkButtonPanel() override;

        zlgui::CompactButton &getButton() { return dyn_link_c_; }

        void updateBound();

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void resized() override;

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlgui::Dragger &side_dragger_ref_;
        zlgui::CompactButton dyn_link_c_;
        float button_size_{}, button_bottom_{};
        std::atomic<bool> button_changed_{false};
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > button_attachments_;
        const std::unique_ptr<juce::Drawable> link_drawable_;

        std::atomic<size_t> band_idx_;
        std::atomic<bool> is_dynamic_on_{false}, is_selected_{false};

        constexpr static std::array kIDs{zlp::dynamicON::ID};
        constexpr static std::array kNAIDs{zlstate::selectedBandIdx::ID};

        void parameterChanged(const juce::String &parameter_id, float new_value) override;
    };
} // zlpanel
