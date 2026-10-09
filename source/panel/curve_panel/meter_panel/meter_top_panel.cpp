// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "meter_top_panel.hpp"

namespace zlpanel {
    MeterTopPanel::MeterTopPanel(zlgui::UIBase& base) :
        base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void MeterTopPanel::paint(juce::Graphics& g) {
        auto bound = getLocalBounds().toFloat();
        g.setColour(base_.getBackgroundColour().withAlpha(.5f));
        g.fillRect(bound);

        g.setFont(base_.getFontSize());
        g.setColour(base_.getTextColour());
        if (out_value_ > -120.f) {
            g.drawText(formatValue(out_value_), bound, juce::Justification::centred, false);
        }
    }

    void MeterTopPanel::updateValue(const float out_value) {
        if (std::abs(out_value - out_value_) > 0.05f) {
            out_value_ = out_value;
            repaint();
        }
    }

    std::string MeterTopPanel::formatValue(const float value) {
        std::stringstream ss;
        const auto abs_value = std::abs(value);
        if (abs_value < 10.f) {
            ss << std::fixed << std::setprecision(2) << value;
        } else if (abs_value < 100.f) {
            ss << std::fixed << std::setprecision(1) << value;
        } else {
            ss << std::fixed << std::setprecision(0) << value;
        }
        return ss.str();
    }
}
