// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "BinaryData.h"

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class ResetComponent final : public juce::Component {
    public:
        explicit ResetComponent(juce::AudioProcessorValueTreeState &parameters,
                                juce::AudioProcessorValueTreeState &parameters_NA,
                                zlgui::UIBase &base);

        ~ResetComponent() override;

        void resized() override;

        void attachGroup(size_t idx);

    private:
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        const std::unique_ptr<juce::Drawable> drawable_;
        zlgui::ClickButton button_;
        std::atomic<size_t> band_idx_;
    };
} // zlpanel
