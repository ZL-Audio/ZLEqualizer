// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CONFLICT_PANEL_HPP
#define ZLEqualizer_CONFLICT_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class ConflictPanel final : public juce::Component {
    public:
        explicit ConflictPanel(zlFFT::ConflictAnalyzer<double> &conflictAnalyzer,
                               zlInterface::UIBase &base);

        ~ConflictPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void updateGradient() {
            if (analyzer.getON()) {
                analyzer.updateGradient(gradient);
                isGradientInit.store(true);
            } else {
                isGradientInit.store(false);
            }
        }

    private:
        zlFFT::ConflictAnalyzer<double> &analyzer;
        zlInterface::UIBase &uiBase;
        juce::Path path;
        juce::ColourGradient gradient;
        std::atomic<bool> isGradientInit{false};
    };
} // zlPanel

#endif //ZLEqualizer_CONFLICT_PANEL_HPP
