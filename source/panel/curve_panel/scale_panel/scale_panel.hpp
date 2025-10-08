// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class ScalePanel final : public juce::Component {
    public:
        explicit ScalePanel(PluginProcessor& p, zlgui::UIBase& base,
                            const multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        static constexpr float kFFTAlpha = .5f;

        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        zlgui::combobox::CompactCombobox eq_max_box_;
        zlgui::attachment::ComboBoxAttachment<true> eq_max_attach_;
        int c_eq_max_idx_{-1};

        zlgui::combobox::CompactCombobox fft_min_box_;
        zlgui::attachment::ComboBoxAttachment<true> fft_min_attach_;
        int c_fft_min_idx_{-1};

        float getUnitHeight() const;
    };
}
