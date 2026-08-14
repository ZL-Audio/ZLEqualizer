// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "group_list.hpp"
#include "../helper/paint_selected_card.hpp"
#include "preset_list_layout.hpp"

namespace zlpanel {
    GroupList::GroupList(zlgui::UIBase& base) :
        zlgui::scrolling::VirtualizedList(base) {
    }

    void GroupList::setGroups(const juce::StringArray& groups, const juce::String& selected_group) {
        groups_ = groups;
        setRowCount(groups_.size());
        setSelectedRow(groups_.indexOf(selected_group));
    }

    void GroupList::paintRow(juce::Graphics& g, const int row, juce::Rectangle<int> bounds,
                             const bool selected, const bool hovered) {
        if (!juce::isPositiveAndBelow(row, groups_.size())) {
            return;
        }

        const auto font_size = base_.getFontSize();
        paintSelectableCard(g, bounds, base_.getTextColour(), font_size, selected, hovered);

        g.setColour(base_.getTextColour().withAlpha(selected ? .95f : .68f));
        g.setFont(juce::FontOptions{1.5f * font_size});
        g.drawFittedText(groups_[row], bounds.reduced(preset_list_layout::rowTextInset(font_size), 0),
                         juce::Justification::centredLeft, 1);
    }

    void GroupList::rowClicked(const int row) {
        if (juce::isPositiveAndBelow(row, groups_.size()) && onGroupSelected) {
            onGroupSelected(groups_[row]);
        }
    }

    void GroupList::rowDoubleClicked(const int row) {
        rowClicked(row);
    }
}
