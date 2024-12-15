// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLPANEL_MATCH_ANALYZER_PANEL_HPP
#define ZLPANEL_MATCH_ANALYZER_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"

namespace zlPanel {
    class MatchAnalyzerPanel final : public juce::Component {
    public:
        explicit MatchAnalyzerPanel(zlEqMatch::EqMatchAnalyzer<double> &analyzer,
                                    zlInterface::UIBase &base);

        ~MatchAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void updatePaths();

    private:
        zlEqMatch::EqMatchAnalyzer<double> &analyzerRef;
        zlInterface::UIBase &uiBase;
        juce::Path path1{}, path2{}, path3{};
        juce::Path recentPath1{}, recentPath2{}, recentPath3{};
        juce::SpinLock pathLock;
        AtomicPoint leftCorner, rightCorner;
        AtomicBound atomicBound;
        std::atomic<bool> firstPath = true;
    };
} // zlPanel

#endif //ZLPANEL_MATCH_ANALYZER_PANEL_HPP
