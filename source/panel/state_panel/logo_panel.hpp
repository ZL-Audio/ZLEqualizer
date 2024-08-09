// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_LOGO_PANEL_HPP
#define ZLEqualizer_LOGO_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {
    class LogoPanel final : public juce::Component {
    public:
        explicit LogoPanel(PluginProcessor &p,
                           zlInterface::UIBase &base);

        void paint(juce::Graphics &g) override;

        void setJustification(int justificationFlags);

    private:
        juce::AudioProcessorValueTreeState &stateRef;
        zlInterface::UIBase &uiBase;
        const std::unique_ptr<juce::Drawable> brandDrawable, logoDrawable;
        juce::Justification justification {juce::Justification::topLeft};
    };
} // zlPanel

#endif //ZLEqualizer_LOGO_PANEL_HPP
