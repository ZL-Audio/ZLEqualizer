// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "piano_band_panel.hpp"

namespace zlpanel {
    PianoBandPanel::PianoBandPanel(zlgui::UIBase& base, const PianoKeysPanel& keys) : base_(base), keys_(keys) {
        notes_.fill(-1);
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    void PianoBandPanel::paint(juce::Graphics& g) {
        for (size_t band = 0; band < regions_.size(); ++band) {
            if (regions_[band].isEmpty()) {
                continue;
            }
            g.setColour(base_.getColourMap1(band).withAlpha(1.f));
            g.fillRectList(regions_[band]);
        }
        if (selected_ < regions_.size()) {
            const auto& region = regions_[selected_];
            const auto bound = region.getBounds();
            const auto underline = bound.withTop(bound.getBottom() - std::max(1.f, base_.getFontSize() * .08f));
            g.setColour(base_.getTextColour().withAlpha(1.f));
            for (const auto& rectangle : region) {
                g.fillRect(rectangle.getIntersection(underline));
            }
        }
    }

    void PianoBandPanel::setBands(const std::array<int, zlp::kBandNum>& notes, const size_t selected,
                                 const bool geometry_changed) {
        if (notes_ == notes && !geometry_changed) {
            setSelectedBand(selected);
            return;
        }
        const auto old_notes = notes_;
        notes_ = notes;
        std::array<bool, zlp::kBandNum> changed{};
        for (size_t band = 0; band < notes_.size(); ++band) {
            if (geometry_changed) {
                changed[band] = true;
            } else if (old_notes[band] != notes_[band]) {
                changed[band] = true;
                for (size_t other = 0; other < notes_.size(); ++other) {
                    if (notes_[other] >= 0
                        && (notes_[other] == old_notes[band] || notes_[other] == notes_[band])) {
                        changed[other] = true;
                    }
                }
            }
        }
        for (size_t band = 0; band < notes_.size(); ++band) {
            if (changed[band]) {
                updateRegion(band);
            }
        }
        setSelectedBand(selected);
    }

    void PianoBandPanel::setBand(const size_t band, const int note) {
        if (notes_[band] == note) {
            return;
        }
        const auto old_note = notes_[band];
        notes_[band] = note;
        for (size_t other = 0; other < notes_.size(); ++other) {
            if (other == band || (notes_[other] >= 0 && (notes_[other] == old_note || notes_[other] == note))) {
                updateRegion(other);
            }
        }
    }

    void PianoBandPanel::setSelectedBand(const size_t selected) {
        if (selected_ != selected) {
            for (const auto band : {selected_, selected}) {
                if (band < regions_.size() && !regions_[band].isEmpty()) {
                    repaint(regions_[band].getBounds().getSmallestIntegerContainer().expanded(1));
                }
            }
            selected_ = selected;
        }
    }

    void PianoBandPanel::updateRegion(const size_t band) {
        auto& region = regions_[band];
        const auto old_bound = region.getBounds();
        region.clear();
        if (const auto* key = notes_[band] >= 0 ? keys_.getKey(notes_[band]) : nullptr) {
            size_t count = 0, position = 0;
            for (size_t other = 0; other < notes_.size(); ++other) {
                if (notes_[other] == notes_[band]) {
                    ++count;
                    position += other < band ? 1 : 0;
                }
            }
            // split coincident bands into independently selectable sections of the same key
            const auto height = key->bounds.getHeight() / static_cast<float>(count);
            auto section = key->bounds.withY(key->bounds.getY() + static_cast<float>(position) * height);
            section.setHeight(height);
            key->region.getIntersectionWith(section, region);
        }
        if (!old_bound.isEmpty()) {
            repaint(old_bound.getSmallestIntegerContainer().expanded(1));
        }
        const auto new_bound = region.getBounds();
        if (!new_bound.isEmpty() && new_bound != old_bound) {
            repaint(new_bound.getSmallestIntegerContainer().expanded(1));
        }
    }

    size_t PianoBandPanel::getBandAt(const juce::Point<float> point) const {
        for (size_t band = 0; band < regions_.size(); ++band) {
            if (regions_[band].containsPoint(point.x, point.y)) {
                return band;
            }
        }
        return zlp::kBandNum;
    }

    void PianoBandPanel::lookAndFeelChanged() {
        repaint();
    }
}
