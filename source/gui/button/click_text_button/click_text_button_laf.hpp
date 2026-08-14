// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <functional>
#include <utility>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlgui {
    class ClickTextButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        using BackgroundPainter = std::function<void(juce::Graphics&, juce::Button&, bool, bool)>;

        explicit ClickTextButtonLookAndFeel(UIBase& base) : base_(base) {
        }

        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                  const juce::Colour&, const bool highlight,
                                  const bool down) override {
            if (background_painter_) {
                background_painter_(g, button, highlight, down);
            }
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                            const bool highlight, const bool down) override {
            if (down || button.getToggleState()) {
                g.setColour(base_.getTextColour());
            } else if (highlight) {
                g.setColour(base_.getTextColour().withAlpha(.75f));
            } else {
                g.setColour(base_.getTextColour().withAlpha(.5f));
            }
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(button.getButtonText(), button.getBounds(), justification_);
        }

        void setJustification(const juce::Justification justification) {
            justification_ = justification;
        }

        void setFontScale(const float font_scale) {
            font_scale_ = font_scale;
        }

        void setBackgroundPainter(BackgroundPainter painter) {
            background_painter_ = std::move(painter);
        }

    private:
        UIBase& base_;

        float font_scale_{1.f};
        juce::Justification justification_{juce::Justification::centredLeft};
        BackgroundPainter background_painter_;
    };
}
