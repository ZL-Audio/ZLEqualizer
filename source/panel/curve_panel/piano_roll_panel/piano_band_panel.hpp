// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "piano_keys_panel.hpp"
#include "../../../zlp/zlp_definitions.hpp"

namespace zlpanel {
    class PianoBandPanel final : public juce::Component {
    public:
        PianoBandPanel(zlgui::UIBase& base, const PianoKeysPanel& keys);

        void paint(juce::Graphics& g) override;

        void setBands(const std::array<int, zlp::kBandNum>& notes, size_t selected, bool geometry_changed = false);

        void setBand(size_t band, int note);

        void setSelectedBand(size_t selected);

        size_t getBandAt(juce::Point<float> point) const;

    private:
        zlgui::UIBase& base_;
        const PianoKeysPanel& keys_;
        std::array<int, zlp::kBandNum> notes_;
        std::array<juce::RectangleList<float>, zlp::kBandNum> regions_;
        size_t selected_{zlp::kBandNum};

        void updateRegion(size_t band);

        void lookAndFeelChanged() override;
    };
}
