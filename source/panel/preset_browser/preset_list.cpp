// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "preset_list.hpp"

#include <algorithm>

#include "../../gui/popup/popup_style.hpp"
#include "preset_list_layout.hpp"

namespace zlpanel {
    PresetList::PresetList(zlgui::UIBase& base) : zlgui::scrolling::VirtualizedList(base) {
    }

    void PresetList::setPresets(const std::vector<PresetEntry>& presets, const juce::File& selected_file,
                                const bool show_groups) {
        presets_ = &presets;
        show_groups_ = show_groups;
        setRowCount(static_cast<int>(presets.size()));
        setSelectedRow(findFile(selected_file));
    }

    void PresetList::selectFile(const juce::File& file, const bool scroll_to_row) {
        const auto row = findFile(file);
        setSelectedRow(row);
        if (scroll_to_row) {
            scrollToRow(row);
        }
    }

    void PresetList::paintRow(juce::Graphics& g, const int row, juce::Rectangle<int> bounds,
                              const bool selected, const bool hovered) {
        if (presets_ == nullptr || !juce::isPositiveAndBelow(row, static_cast<int>(presets_->size()))) {
            return;
        }

        const auto& preset = (*presets_)[static_cast<size_t>(row)];
        const auto font_size = getBase().getFontSize();
        zlgui::popup::paintSelectableCard(g, bounds, getBase().getTextColour(), font_size,
                                          selected, hovered);

        auto text_bounds = bounds.reduced(preset_list_layout::rowTextInset(font_size), 0);
        g.setFont(juce::FontOptions{zlgui::popup::textFontSize(font_size)});
        if (show_groups_) {
            auto group_bounds = text_bounds.removeFromRight(juce::jmin(text_bounds.getWidth() / 3,
                                                                       juce::roundToInt(font_size * 10.f)));
            g.setColour(getBase().getTextHideColour());
            g.drawFittedText(preset.group, group_bounds, juce::Justification::centredRight, 1);
            text_bounds.removeFromRight(juce::roundToInt(font_size * .5f));
        }
        g.setColour(getBase().getTextColour().withAlpha(selected ? .95f : .7f));
        g.drawFittedText(preset.name, text_bounds, juce::Justification::centredLeft, 1);
    }

    void PresetList::rowClicked(const int row) {
        if (presets_ != nullptr && juce::isPositiveAndBelow(row, static_cast<int>(presets_->size())) &&
            onPresetSelected) {
            onPresetSelected((*presets_)[static_cast<size_t>(row)].file);
        }
    }

    void PresetList::rowDoubleClicked(const int row) {
        if (presets_ == nullptr || !juce::isPositiveAndBelow(row, static_cast<int>(presets_->size()))) {
            return;
        }
        const auto& file = (*presets_)[static_cast<size_t>(row)].file;
        if (onPresetSelected) {
            onPresetSelected(file);
        }
        if (onPresetLoad) {
            onPresetLoad(file);
        }
    }

    int PresetList::findFile(const juce::File& file) const {
        if (presets_ == nullptr || file == juce::File{}) {
            return -1;
        }
        const auto iterator = std::find_if(presets_->begin(), presets_->end(), [&file](const auto& preset) {
            return preset.file == file;
        });
        return iterator == presets_->end()
                   ? -1
                   : static_cast<int>(std::distance(presets_->begin(), iterator));
    }
}
