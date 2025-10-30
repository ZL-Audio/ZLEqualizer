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

#include "../control_panel/control_background.hpp"

namespace zlpanel {
    class AnalyzerLabel final : public juce::Component,
                              private juce::ValueTree::Listener {
    public:
        explicit AnalyzerLabel(PluginProcessor&, zlgui::UIBase& base);

        ~AnalyzerLabel() override;

        void resized() override;

        void mouseDown(const juce::MouseEvent&) override;

    private:
        zlgui::UIBase& base_;

        ControlBackground control_background_;

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label analyzer_label_;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
