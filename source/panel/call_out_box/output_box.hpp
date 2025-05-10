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
    class OutputBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit OutputBox(PluginProcessor &p,
                                  zlgui::UIBase &base)
            : processor_ref_(p), parameters_ref_(p.parameters_),
              ui_base_(base),
              phase_c_("phase", ui_base_, zlgui::multilingual::Labels::kPhaseFlip),
              agc_c_("A", ui_base_, zlgui::multilingual::Labels::kAutoGC),
              lm_c_("L", ui_base_, zlgui::multilingual::Labels::kLoudnessMatch),
              scale_s_("Scale", ui_base_, zlgui::multilingual::Labels::kScale),
              out_gain_s_("Out Gain", ui_base_, zlgui::multilingual::Labels::kOutputGain),
              phase_drawable_(
                  juce::Drawable::createFromImageData(BinaryData::fadphase_svg,
                                                      BinaryData::fadphase_svgSize)),
              agc_drawable_(juce::Drawable::createFromImageData(BinaryData::autogaincompensation_svg,
                                                              BinaryData::autogaincompensation_svgSize)),
              lm_drawable_(juce::Drawable::createFromImageData(BinaryData::loudnessmatch_svg,
                                                             BinaryData::loudnessmatch_svgSize)),
              agc_updater_(p.parameters_, zlp::autoGain::ID),
              gain_updater_(p.parameters_, zlp::outputGain::ID) {
            setBufferedToImage(true);
            phase_c_.setDrawable(phase_drawable_.get());
            agc_c_.setDrawable(agc_drawable_.get());
            lm_c_.setDrawable(lm_drawable_.get());

            for (auto &c: {&phase_c_, &agc_c_, &lm_c_}) {
                c->getLAF().enableShadow(false);
                c->getLAF().setShrinkScale(0.f);
                addAndMakeVisible(c);
            }
            for (auto &c: {&scale_s_, &out_gain_s_}) {
                addAndMakeVisible(c);
            }
            attach({&phase_c_.getButton(), &agc_c_.getButton(), &lm_c_.getButton()},
                   {zlp::phaseFlip::ID, zlp::autoGain::ID, zlp::loudnessMatcherON::ID},
                   parameters_ref_, button_attachments_);
            attach({&scale_s_.getSlider(), &out_gain_s_.getSlider()},
                   {zlp::scale::ID, zlp::outputGain::ID},
                   parameters_ref_, slider_attachments_);

            lm_c_.getButton().onClick = [this]() {
                if (!lm_c_.getButton().getToggleState()) {
                    const auto newGain = -processor_ref_.getController().getLoudnessMatcherDiff();
                    agc_updater_.updateSync(0.f);
                    gain_updater_.updateSync(zlp::outputGain::convertTo01(static_cast<float>(newGain)));
                }
            };

            ui_base_.getBoxTree().addListener(this);
        }

        ~OutputBox() override {
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
            const auto button_height = static_cast<int>(kButtonHeightP * ui_base_.getFontSize());
            const auto button_width = static_cast<int>(ui_base_.getFontSize() * 2.5);
            return {button_width * 3 + pad_size * 2, button_height * 3 + pad_size};
        }

        void resized() override {
            for (auto &c: {&scale_s_, &out_gain_s_}) {
                c->setPadding(std::round(ui_base_.getFontSize() * 0.5f),
                              std::round(ui_base_.getFontSize() * 0.6f));
            }

            const auto pad_size = juce::roundToInt(ui_base_.getFontSize() * 0.5f);
            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + pad_size, bound.getY(),
                                         bound.getWidth() - pad_size * 2, bound.getHeight() - pad_size);

            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50)), Track(Fr(50)), Track(Fr(50))};

            grid.items = {
                juce::GridItem(scale_s_).withArea(1, 1, 2, 4),
                juce::GridItem(phase_c_).withArea(2, 1),
                juce::GridItem(agc_c_).withArea(2, 2),
                juce::GridItem(lm_c_).withArea(2, 3),
                juce::GridItem(out_gain_s_).withArea(3, 1, 4, 4)
            };

            grid.performLayout(bound);
        }

    private:
        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_;
        zlgui::UIBase &ui_base_;

        zlgui::CompactButton phase_c_, agc_c_, lm_c_;
        juce::OwnedArray<zlgui::ButtonCusAttachment<false> > button_attachments_{};

        zlgui::CompactLinearSlider scale_s_, out_gain_s_;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> slider_attachments_{};

        const std::unique_ptr<juce::Drawable> phase_drawable_;
        const std::unique_ptr<juce::Drawable> agc_drawable_;
        const std::unique_ptr<juce::Drawable> lm_drawable_;

        zldsp::chore::ParaUpdater agc_updater_, gain_updater_;

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(tree_whose_property_has_changed, property);
            if (ui_base_.isBoxProperty(zlgui::BoxIdx::kOutputBox, property)) {
                const auto f = static_cast<bool>(ui_base_.getBoxProperty(zlgui::BoxIdx::kOutputBox));
                setVisible(f);
            }
        }
    };
}
