// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"
#include "../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class LogoPanel final : public juce::Component,
                            public juce::SettableTooltipClient {
    public:
        explicit LogoPanel(PluginProcessor& p, zlgui::UIBase& base,
                           multilingual::TooltipHelper& tooltip_helper);

    private:
        zlgui::UIBase& base_;
        std::unique_ptr<juce::Drawable> brand_drawable_, logo_drawable_;

        void paint(juce::Graphics& g) override;

        void mouseEnter(const juce::MouseEvent& event) override;

        void mouseExit(const juce::MouseEvent& event) override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;
    };
}
