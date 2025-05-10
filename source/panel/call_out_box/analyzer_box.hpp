// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlpanel {
    class AnalyzerBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit AnalyzerBox(juce::AudioProcessorValueTreeState &parameters_NA,
                               zlgui::UIBase &base)
            : parameters_NA_ref_(parameters_NA),
              ui_base_(base),
              fft_pre_on_("Pre:", zlstate::fftPreON::choices, ui_base_, zlgui::multilingual::Labels::kFFTPre),
              fft_post_on_("Post:", zlstate::fftPostON::choices, ui_base_, zlgui::multilingual::Labels::kFFTPost),
              fft_side_on_("Side:", zlstate::fftSideON::choices, ui_base_, zlgui::multilingual::Labels::kFFTSide),
              fft_speed_("", zlstate::ffTSpeed::choices, ui_base_, zlgui::multilingual::Labels::kFFTDecay),
              fft_tilt_("", zlstate::ffTTilt::choices, ui_base_, zlgui::multilingual::Labels::kFFTSlope) {
            for (auto &c: {&fft_pre_on_, &fft_post_on_, &fft_side_on_}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlgui::ClickCombobox::kLeft);
                addAndMakeVisible(c);
            }
            for (auto &c: {&fft_speed_, &fft_tilt_}) {
                addAndMakeVisible(c);
            }
            attach({
                       &fft_pre_on_.getCompactBox().getBox(),
                       &fft_post_on_.getCompactBox().getBox(),
                       &fft_side_on_.getCompactBox().getBox(),
                       &fft_speed_.getBox(), &fft_tilt_.getBox()
                   },
                   {
                       zlstate::fftPreON::ID, zlstate::fftPostON::ID, zlstate::fftSideON::ID,
                       zlstate::ffTSpeed::ID, zlstate::ffTTilt::ID
                   },
                   parameters_NA_ref_, box_attachments_);
            setBufferedToImage(true);

            ui_base_.getBoxTree().addListener(this);
        }

        ~AnalyzerBox() override {
            ui_base_.getBoxTree().removeListener(this);
        }

        void paint(juce::Graphics &g) override {
            juce::Path path;
            const auto bound = getLocalBounds().toFloat();
            path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                     std::round(ui_base_.getFontSize() * .25f),
                                     std::round(ui_base_.getFontSize() * .25f),
                                     false, false, true, true);
            g.setColour(ui_base_.getBackgroundColor());
            g.fillPath(path);
        }

        juce::Rectangle<int> getIdealBound() const {
            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            const auto button_width = static_cast<int>(ui_base_.getFontSize() * 2.5);
            const auto box_height = juce::roundToInt(kBoxHeightP * ui_base_.getFontSize());
            return {button_width * 3 + pad_size * 2, box_height * 5 + pad_size};
        }

        void resized() override {
            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.25f);
            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + pad_size, bound.getY(),
                                         bound.getWidth() - pad_size * 2, bound.getHeight() - pad_size);

            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(fft_pre_on_).withArea(1, 1),
                juce::GridItem(fft_post_on_).withArea(2, 1),
                juce::GridItem(fft_side_on_).withArea(3, 1),
                juce::GridItem(fft_speed_).withArea(4, 1),
                juce::GridItem(fft_tilt_).withArea(5, 1)
            };
            grid.performLayout(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::ClickCombobox fft_pre_on_, fft_post_on_, fft_side_on_;
        zlgui::CompactCombobox fft_speed_, fft_tilt_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> box_attachments_{};

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(tree_whose_property_has_changed);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kAnalyzerBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kAnalyzerBox));
                setVisible(f);
            }
        }
    };
}
