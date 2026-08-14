// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../gui/interface_definitions.hpp"

namespace zlpanel {
    class PanelBackground final : public juce::Component {
    public:
        explicit PanelBackground(zlgui::UIBase& base, float shadow_alpha = .5f);

        void paint(juce::Graphics& g) override;

        void setSurfaceBounds(std::vector<juce::Rectangle<int>> bounds);

    private:
        zlgui::UIBase& base_;
        const float shadow_alpha_;
        std::vector<juce::Rectangle<int>> surface_bounds_;
        bool paints_surfaces_ = false;
    };
}
