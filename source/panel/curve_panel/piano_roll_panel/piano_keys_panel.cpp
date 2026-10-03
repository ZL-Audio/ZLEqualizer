// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "piano_keys_panel.hpp"
#include "../../helper/freq_helper.hpp"
#include "../../helper/piano_roll_helper.hpp"

namespace zlpanel {
    namespace {
        constexpr std::array kWhiteNotes{0, 2, 4, 5, 7, 9, 11};
        constexpr std::array kWhiteIndices{0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};
        constexpr double kWhiteKeyWidth = 12.0 / 7.0;
        constexpr double kWhiteKeyOffset = 4.0 / 7.0;
    }

    PianoKeysPanel::PianoKeysPanel(zlgui::UIBase& base) : base_(base) {
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    void PianoKeysPanel::paint(juce::Graphics& g) {
        const auto bound = getLocalBounds().toFloat();
        if (bound.isEmpty() || fft_max_ <= piano_roll_helper::kMinFrequency) {
            return;
        }
        const auto width = bound.getWidth();
        const auto last_note = first_note_ + static_cast<int>(keys_.size()) - 1;
        const auto thickness = std::max(1.f, std::round(base_.getFontSize() * .06f));
        juce::RectangleList<float> black_keys, lines;
        for (auto note = first_note_; note <= last_note; ++note) {
            if (piano_roll_helper::isBlackKey(note)) {
                black_keys.add(getBlackKeyBounds(note).toNearestInt().toFloat());
            } else {
                const auto x = getWhiteKeyBounds(note).getRight();
                lines.add(juce::Rectangle<float>(std::round(x - thickness * .5f), 0.f,
                                                 thickness, bound.getHeight()));
            }
        }
        black_keys.clipTo(bound);
        lines.clipTo(bound);
        lines.subtract(black_keys);
        lines.add(juce::Rectangle<float>(0.f, 0.f, width, thickness));
        lines.add(juce::Rectangle<float>(0.f, bound.getBottom() - thickness, width, thickness));
        lines.add(juce::Rectangle<float>(0.f, 0.f, thickness, bound.getHeight()));
        lines.add(juce::Rectangle<float>(width - thickness, 0.f, thickness, bound.getHeight()));
        black_keys.subtract(lines);
        juce::RectangleList<float> white_keys(bound);
        white_keys.subtract(black_keys);
        white_keys.subtract(lines);

        const auto top_fade = std::max(2.f, std::min(bound.getHeight(), base_.getFontSize() * .65f));
        const auto white_colour = base_.getBackgroundColour().interpolatedWith(juce::Colours::white, .6f);
        const auto black_colour = base_.getBackgroundColour().interpolatedWith(juce::Colours::black, .65f);
        const auto line_colour = base_.getBackgroundColour().interpolatedWith(base_.getTextColour(), .3f);
        const auto draw_region = [&](const juce::RectangleList<float>& region, const juce::Colour colour) {
            g.setGradientFill({colour.withAlpha(0.f), 0.f, 1.f,
                               colour.withMultipliedAlpha(.8f), 0.f, top_fade, false});
            g.fillRectList(region);
        };
        draw_region(white_keys, white_colour);
        draw_region(black_keys, black_colour);
        draw_region(lines, line_colour);
    }

    void PianoKeysPanel::resized() {
        updateGeometry();
    }

    void PianoKeysPanel::updateSampleRate(const double sample_rate) {
        fft_max_ = freq_helper::getFFTMax(sample_rate);
        updateGeometry();
        repaint();
    }

    const PianoKeysPanel::Key* PianoKeysPanel::getKey(const int note) const {
        const auto index = note - first_note_;
        return index >= 0 && index < static_cast<int>(keys_.size())
            ? &keys_[static_cast<size_t>(index)] : nullptr;
    }

    int PianoKeysPanel::getNoteAt(const juce::Point<float> point) const {
        if (keys_.empty() || !getLocalBounds().toFloat().contains(point)) {
            return -1;
        }
        const auto position = xToNote(point.x);
        const auto nearest = static_cast<int>(std::round(position));
        if (piano_roll_helper::isBlackKey(nearest) && getBlackKeyBounds(nearest).contains(point)) {
            return getKey(nearest) != nullptr ? nearest : -1;
        }
        const auto white_index = static_cast<int>(std::floor((position + kWhiteKeyOffset) / kWhiteKeyWidth));
        const auto pitch_index = (white_index % 7 + 7) % 7;
        const auto octave = (white_index - pitch_index) / 7;
        const auto note = octave * 12 + kWhiteNotes[static_cast<size_t>(pitch_index)];
        return getKey(note) != nullptr ? note : -1;
    }

    float PianoKeysPanel::noteToX(const double note) const {
        return a4_x_ + static_cast<float>(note - 69.0) * note_width_;
    }

    double PianoKeysPanel::xToNote(const float x) const {
        return note_width_ > 0.f ? 69.0 + static_cast<double>((x - a4_x_) / note_width_) : 0.0;
    }

    juce::Rectangle<float> PianoKeysPanel::getBlackKeyBounds(const int note) const {
        return {noteToX(note) - note_width_ * .4f, 0.f, note_width_ * .8f,
                static_cast<float>(getHeight()) * .6f};
    }

    juce::Rectangle<float> PianoKeysPanel::getWhiteKeyBounds(const int note) const {
        const auto pitch_class = (note % 12 + 12) % 12;
        const auto octave = (note - pitch_class) / 12;
        const auto index = octave * 7 + kWhiteIndices[static_cast<size_t>(pitch_class)];
        const auto left = noteToX(static_cast<double>(index) * kWhiteKeyWidth - kWhiteKeyOffset);
        const auto right = noteToX(static_cast<double>(index + 1) * kWhiteKeyWidth - kWhiteKeyOffset);
        return {left, 0.f, right - left, static_cast<float>(getHeight())};
    }

    void PianoKeysPanel::updateGeometry() {
        keys_.clear();
        note_width_ = 0.f;
        if (getLocalBounds().isEmpty() || fft_max_ <= piano_roll_helper::kMinFrequency) {
            return;
        }
        const auto width = static_cast<float>(getWidth());
        a4_x_ = piano_roll_helper::frequencyToX(440.0, width, fft_max_);
        note_width_ = piano_roll_helper::frequencyToX(piano_roll_helper::noteToFrequency(70.0), width, fft_max_) - a4_x_;
        first_note_ = static_cast<int>(std::floor(xToNote(0.f))) - 1;
        const auto last_note = static_cast<int>(std::ceil(xToNote(width))) + 1;
        keys_.resize(static_cast<size_t>(last_note - first_note_ + 1));
        const auto thickness = std::max(.5f, base_.getFontSize() * .06f);
        const auto interior = getLocalBounds().toFloat().reduced(thickness);
        for (auto note = first_note_; note <= last_note; ++note) {
            auto& key = keys_[static_cast<size_t>(note - first_note_)];
            if (piano_roll_helper::isBlackKey(note)) {
                key.bounds = getBlackKeyBounds(note);
                key.region.add(key.bounds);
            } else {
                key.bounds = getWhiteKeyBounds(note);
                key.region.add(key.bounds.reduced(thickness * .5f, 0.f));
                for (const auto neighbour : {note - 1, note + 1}) {
                    if (piano_roll_helper::isBlackKey(neighbour)) {
                        key.region.subtract(getBlackKeyBounds(neighbour));
                    }
                }
            }
            key.region.clipTo(interior);
        }
    }

    void PianoKeysPanel::lookAndFeelChanged() {
        updateGeometry();
        repaint();
    }
}
