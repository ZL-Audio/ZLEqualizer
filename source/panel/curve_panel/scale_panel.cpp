// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "scale_panel.hpp"
#include "../panel_definitons.hpp"

namespace zlpanel {
    ScalePanel::ScalePanel(PluginProcessor &processor, zlgui::UIBase &base)
        : parameters_NA_ref_(processor.parameters_NA_), ui_base_(base),
          scale_box_("", zlstate::maximumDB::choices, base),
          min_fft_box_("", zlstate::minimumFFTDB::choices, base) {
        juce::ignoreUnused(ui_base_);
        scale_box_.getLAF().setFontScale(zlgui::kFontLarge);
        scale_box_.getBox().setJustificationType(juce::Justification::centredRight);
        scale_box_.getBox().onChange = [this]() {
            handleAsyncUpdate();
        };
        min_fft_box_.getLAF().setFontScale(zlgui::kFontLarge);
        min_fft_box_.getBox().setJustificationType(juce::Justification::centredRight);
        min_fft_box_.getBox().onChange = [this]() {
            handleAsyncUpdate();
        };
        attach({&scale_box_.getBox(), &min_fft_box_.getBox()},
               {zlstate::maximumDB::ID, zlstate::minimumFFTDB::ID},
               parameters_NA_ref_, box_attachments_);
        addAndMakeVisible(scale_box_);
        addAndMakeVisible(min_fft_box_);
        handleAsyncUpdate();

        SettableTooltipClient::setTooltip(ui_base_.getToolTipText(zlgui::multilingual::Labels::kDBScale));

        setBufferedToImage(true);
    }

    ScalePanel::~ScalePanel() = default;

    void ScalePanel::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getBackgroundColor());

        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * ui_base_.getFontSize());

        g.setFont(ui_base_.getFontSize() * zlgui::kFontLarge);
        for (auto &d: kScaleDBs) {
            const auto y = d * bound.getHeight() + bound.getY() - ui_base_.getFontSize() * .75f;
            const auto l_text_bound = juce::Rectangle<float>(bound.getX(), y, bound.getWidth() * .4f,
                                                           ui_base_.getFontSize() * 1.5f);
            const auto left_db = static_cast<int>(std::round(-2 * d * maximum_db_.load() + maximum_db_.load()));
            g.setColour(ui_base_.getTextColor());
            g.drawText(juce::String(left_db), l_text_bound, juce::Justification::centredRight);
            const auto r_text_bound = juce::Rectangle<float>(bound.getCentreX(), y, bound.getWidth() * .4f,
                                                           ui_base_.getFontSize() * 1.5f);
            const auto fft_db = static_cast<int>(std::round(minimum_fft_db_.load() * d));
            if (fft_db > -100) {
                g.setColour(ui_base_.getTextInactiveColor());
                g.drawText(juce::String(fft_db), r_text_bound, juce::Justification::centredRight);
            }
        }
    }

    void ScalePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * ui_base_.getFontSize());
        {
            auto box_bound = juce::Rectangle<float>(ui_base_.getFontSize() * 4.f, ui_base_.getFontSize() * 1.5f);
            box_bound = box_bound.withCentre({bound.getCentreX(), bound.getY()});
            box_bound.removeFromRight(ui_base_.getFontSize() * .95f);
            box_bound.removeFromLeft(ui_base_.getFontSize() * .05f);
            scale_box_.setBounds(box_bound.toNearestInt());
        }
        {
            auto box_bound = juce::Rectangle<float>(bound.getWidth(), ui_base_.getFontSize() * 1.5f);
            box_bound = box_bound.withCentre({bound.getCentreX(), bound.getBottom()});
            box_bound.removeFromRight(ui_base_.getFontSize() * .5f);
            box_bound.removeFromLeft(ui_base_.getFontSize() * .5f);
            min_fft_box_.setBounds(box_bound.toNearestInt());
        }
    }

    void ScalePanel::handleAsyncUpdate() {
        maximum_db_.store(zlstate::maximumDB::dBs[
            static_cast<size_t>(parameters_NA_ref_.getRawParameterValue(zlstate::maximumDB::ID)->load())]);
        minimum_fft_db_.store(zlstate::minimumFFTDB::dBs[
            static_cast<size_t>(parameters_NA_ref_.getRawParameterValue(zlstate::minimumFFTDB::ID)->load())]);
        repaint();
    }
} // zlpanel
