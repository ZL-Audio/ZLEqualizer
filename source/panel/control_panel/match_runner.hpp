// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"
#include "../../dsp/eq_match/eq_match_optimizer.hpp"

namespace zlpanel {
    class MatchRunner final : public juce::Thread,
                              private juce::AsyncUpdater,
                              private juce::Slider::Listener {
    public:
        explicit MatchRunner(PluginProcessor& p, zlgui::UIBase& base,
                             zlgui::slider::CompactLinearSlider<false, false, false>& num_band_slider);

        ~MatchRunner() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::slider::CompactLinearSlider<false, false, false>& num_band_slider_;
        std::mutex mutex_;

        size_t suggest_num_band_{0};
        std::vector<zldsp::filter::FilterParameters> paras_{};
        std::vector<zldsp::filter::FilterParameters> copy_paras_{};

        void run() override;

        void handleAsyncUpdate() override;

        void sliderValueChanged(juce::Slider*) override;
    };
}
