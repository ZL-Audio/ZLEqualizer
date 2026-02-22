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
    class OutputLabel final : public juce::Component,
                              private juce::ValueTree::Listener {
    public:
        explicit OutputLabel(PluginProcessor& p, zlgui::UIBase& base);

        ~OutputLabel() override;

        void resized() override;

        void repaintCallbackSlow();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        ControlBackground control_background_;

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label gain_label_;
        juce::Label scale_label_;

        int repaint_count_{10};
        std::atomic<float>& scale_ref_;
        float c_scale_{1000.0};

        double c_gain_db_{1000.0};

        void checkUpdate();

        static std::string floatToStringSnprintf(const float value, const int precision = 1) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
            return std::string(buffer);
        }

        bool is_over_{false};

        void mouseDown(const juce::MouseEvent&) override;

        void mouseEnter(const juce::MouseEvent&) override;

        void mouseExit(const juce::MouseEvent&) override;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;

        void updateAlpha(bool is_panel_open);
    };
}
