// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "meter_background_panel.hpp"
#include "meter_display_panel.hpp"

namespace zlpanel {
    class MeterPanel final : public juce::Component {
    public:
        explicit MeterPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~MeterPanel() override;

        MeterDisplayPanel& getDisplayPanel() {
            return meter_display_panel_;
        }

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase& base_;

        MeterBackgroundPanel meter_background_panel_;
        MeterDisplayPanel meter_display_panel_;
    };
}
