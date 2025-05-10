// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "state_panel.hpp"

namespace zlpanel {
    StatePanel::StatePanel(PluginProcessor &p,
                           zlgui::UIBase &base,
                           UISettingPanel &uiSettingPanel)
        : ui_base_(base), parameters_NA_ref_(p.parameters_NA_),
          output_value_panel_(p, base),
          output_setting_panel_(p, base, "", zlgui::BoxIdx::kOutputBox),
          analyzer_setting_panel_(p, base, "Analyzer", zlgui::BoxIdx::kAnalyzerBox),
          dynamic_setting_panel_(p, base, "Dynamic", zlgui::BoxIdx::kDynamicBox),

          collision_setting_panel_(p, base, "Collision", zlgui::BoxIdx::kCollisionBox),
          general_setting_panel_(p, base, "General", zlgui::BoxIdx::kGeneralBox),
          match_setting_panel_(base),
          logo_panel_(p, base, uiSettingPanel),
          effect_c_("", ui_base_, zlgui::multilingual::Labels::kBypass),
          side_c_("", ui_base_, zlgui::multilingual::Labels::kExternalSideChain),
          sgc_c_("", ui_base_, zlgui::multilingual::Labels::kStaticGC),
          effect_drawable_(
              juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg,
                                                  BinaryData::fadpowerswitch_svgSize)),
          side_drawable_(juce::Drawable::createFromImageData(BinaryData::externalside_svg,
                                                           BinaryData::externalside_svgSize)),
          sgc_drawable_(juce::Drawable::createFromImageData(BinaryData::staticgaincompensation_svg,
                                                          BinaryData::staticgaincompensation_svgSize)) {
        setInterceptsMouseClicks(false, true);

        addAndMakeVisible(output_setting_panel_);
        addAndMakeVisible(output_value_panel_);
        addAndMakeVisible(analyzer_setting_panel_);
        addAndMakeVisible(dynamic_setting_panel_);
        addAndMakeVisible(collision_setting_panel_);
        addAndMakeVisible(general_setting_panel_);
        addAndMakeVisible(match_setting_panel_);
        addAndMakeVisible(logo_panel_);

        effect_c_.setDrawable(effect_drawable_.get());
        side_c_.setDrawable(side_drawable_.get());
        sgc_c_.setDrawable(sgc_drawable_.get());

        for (auto &c: {&effect_c_, &side_c_, &sgc_c_}) {
            c->getLAF().enableShadow(false);
            c->getLAF().setShrinkScale(.0f);
            addAndMakeVisible(c);
            c->setBufferedToImage(true);
        }

        attach({
                   &effect_c_.getButton(),
                   &side_c_.getButton(),
                   &sgc_c_.getButton(),
               },
               {zlp::effectON::ID, zlp::sideChain::ID, zlp::staticAutoGain::ID},
               p.parameters_, button_attachments_);

        side_c_.getButton().onClick = [this]() {
            const auto isSideOn = static_cast<int>(side_c_.getButton().getToggleState());
            const auto para = parameters_NA_ref_.getParameter(zlstate::fftSideON::ID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(zlstate::fftSideON::convertTo01(isSideOn));
            para->endChangeGesture();
        };
    }

    void StatePanel::resized() {
        auto bound = getLocalBounds();
        const auto logo_bound = bound.removeFromLeft(
            juce::roundToInt(static_cast<float>(bound.getWidth()) * .125f));
        logo_panel_.setBounds(logo_bound);

        const auto height = static_cast<float>(bound.getHeight());

        const auto button_width = static_cast<int>(ui_base_.getFontSize() * 2.5);
        const auto effect_bound = bound.removeFromRight(button_width);
        effect_c_.setBounds(effect_bound);

        const auto side_bound = bound.removeFromRight(button_width);
        side_c_.setBounds(side_bound);

        const auto sgc_bound = bound.removeFromRight(button_width);
        sgc_c_.setBounds(sgc_bound);

        bound.removeFromRight(button_width / 4);

        bound.removeFromBottom(juce::roundToInt(ui_base_.getFontSize() * .5f));

        const auto label_width = juce::roundToInt(height * kLabelSize);
        const auto gap_width = juce::roundToInt(height * .5f);
        const auto new_bound = bound.removeFromRight(label_width);
        output_value_panel_.setBounds(new_bound);
        output_setting_panel_.setBounds(new_bound);
        bound.removeFromRight(gap_width);
        analyzer_setting_panel_.setBounds(bound.removeFromRight(label_width));
        bound.removeFromRight(gap_width);
        dynamic_setting_panel_.setBounds(bound.removeFromRight(label_width));
        bound.removeFromRight(gap_width);
        collision_setting_panel_.setBounds(bound.removeFromRight(label_width));
        bound.removeFromRight(gap_width);
        general_setting_panel_.setBounds(bound.removeFromRight(label_width));
        bound.removeFromRight(gap_width);
        match_setting_panel_.setBounds(bound.removeFromRight(label_width));
    }
} // zlpanel
