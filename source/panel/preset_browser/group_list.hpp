// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <functional>

#include "../../gui/scrolling/virtualized_list.hpp"

namespace zlpanel {
    class GroupList final : public zlgui::scrolling::VirtualizedList {
    public:
        explicit GroupList(zlgui::UIBase& base);

        void setGroups(const juce::StringArray& groups, const juce::String& selected_group);

        std::function<void(const juce::String&)> onGroupSelected;

    private:
        juce::StringArray groups_;

        void paintRow(juce::Graphics& g, int row, juce::Rectangle<int> bounds,
                      bool selected, bool hovered) override;

        void rowClicked(int row) override;

        void rowDoubleClicked(int row) override;
    };
}
