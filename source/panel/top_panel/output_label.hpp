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
    class OutputLabel final : public juce::Component {
    public:
        explicit OutputLabel(PluginProcessor& p, zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void repaintCallbackSlow();

        void mouseDown(const juce::MouseEvent&) override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        int repaint_count_{10};
        std::atomic<float>& scale_ref_;
        float c_scale_{100.0};

        double c_gain_db_{0.0};

        void checkUpdate();

        static std::string floatToStringSnprintf(const float value, const int precision = 1) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
            return std::string(buffer);
        }
    };
}
