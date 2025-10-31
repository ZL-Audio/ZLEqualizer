// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "right_click_panel.hpp"

namespace zlpanel {
    RightClickPanel::RightClickPanel(PluginProcessor& p, zlgui::UIBase& base,
                                     const multilingual::TooltipHelper&) :
        p_ref_(p), base_(base),
        items_set_(base.getSelectedBandSet()),
        control_background_(base, .25f),
        invert_button_(base, "Invert Gain"),
        lr_split_button_(base, "Split L/R"),
        ms_split_button_(base, "Split M/S"),
        copy_button_(base, "Copy"),
        paste_button_(base, "Paste") {
        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);
        addAndMakeVisible(mouse_event_eater_);

        invert_button_.getButton().onClick = [this]() {
            invertGain();
        };

        lr_split_button_.getButton().onClick = [this]() {
            splitBand(zlp::FilterStereo::kLeft, zlp::FilterStereo::kRight);
        };

        ms_split_button_.getButton().onClick = [this]() {
            splitBand(zlp::FilterStereo::kMid, zlp::FilterStereo::kSide);
        };

        copy_button_.getButton().onClick = [this]() {
            copyBand();
        };

        paste_button_.getButton().onClick = [this]() {
            pasteBand();
        };

        for (auto& b : {&invert_button_, &lr_split_button_, &ms_split_button_,
                        &copy_button_, &paste_button_}) {
            b->getLAF().setFontScale(1.5f);
            b->getLAF().setJustification(juce::Justification::centredLeft);
            addAndMakeVisible(b);
        }

        setInterceptsMouseClicks(false, true);
    }

    int RightClickPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);

        return 4 * padding + slider_width;
    }

    int RightClickPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_height = getButtonSize(font_size);

        return 2 * padding + 5 * button_height;
    }

    void RightClickPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_height = getButtonSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);
        bound.reduce(2 * padding, padding);
        mouse_event_eater_.setBounds(bound);

        for (auto& b : {&invert_button_, &lr_split_button_, &ms_split_button_,
                        &copy_button_, &paste_button_}) {
            b->setBounds(bound.removeFromTop(button_height));
        }
    }

    void RightClickPanel::setPosition(juce::Point<float> pos) {
        const auto parent_width = static_cast<float>(getParentWidth());
        const auto parent_height = static_cast<float>(getParentHeight());

        const auto width = static_cast<float>(getWidth());
        const auto height = static_cast<float>(getHeight());

        if (pos.x + width > parent_width || pos.y + height > parent_height) {
            setTransform(juce::AffineTransform::translation(pos.x - width, pos.y - height));
        } else {
            setTransform(juce::AffineTransform::translation(pos.x, pos.y));
        }
    }

    void RightClickPanel::visibilityChanged() {
        if (isVisible()) {
            const auto f = base_.getSelectedBand() < zlp::kBandNum;
            for (auto& b : {&invert_button_, &lr_split_button_, &ms_split_button_, &copy_button_}) {
                b->setInterceptsMouseClicks(f, f);
                b->setAlpha(f ? 1.f : .25f);
            }
        }
    }

    void RightClickPanel::invertGain() {
        const auto band = base_.getSelectedBand();
        if (band == zlp::kBandNum) {
            return;
        }
        auto* para1 = p_ref_.parameters_.getParameter(zlp::PGain::kID + std::to_string(band));
        updateValue(para1, 1.f - para1->getValue());
        auto* para2 = p_ref_.parameters_.getParameter(zlp::PTargetGain::kID + std::to_string(band));
        updateValue(para2, 1.f - para2->getValue());
        setVisible(false);
    }

    void RightClickPanel::splitBand(zlp::FilterStereo stereo1, zlp::FilterStereo stereo2) {
        const auto band1 = base_.getSelectedBand();
        if (band1 == zlp::kBandNum) {
            return;
        }
        const auto band2 = band_helper::findOffBand(p_ref_);
        if (band2 == zlp::kBandNum) {
            return;
        }
        const auto band1_s = std::to_string(band1);
        const auto band2_s = std::to_string(band2);
        for (auto& ID : kIDs) {
            if (ID != zlp::PLRMode::kID) {
                auto* para1 = p_ref_.parameters_.getParameter(ID + band1_s);
                auto* para2 = p_ref_.parameters_.getParameter(ID + band2_s);
                updateValue(para2, para1->getValue());
            }
        }
        auto* para1 = p_ref_.parameters_.getParameter(zlp::PLRMode::kID + band1_s);
        updateValue(para1, para1->convertTo0to1(static_cast<float>(stereo1)));
        auto* para2 = p_ref_.parameters_.getParameter(zlp::PLRMode::kID + band2_s);
        updateValue(para2, para2->convertTo0to1(static_cast<float>(stereo2)));

        setVisible(false);
    }

    void RightClickPanel::copyBand() {
        if (base_.getSelectedBand() == zlp::kBandNum) {
            return;
        }
        setVisible(false);

        juce::ValueTree tree{"filter_info"};
        auto selected_band = items_set_.getItemArray();
        if (selected_band.isEmpty()) {
            selected_band.add(base_.getSelectedBand());
        }

        int i = 0;
        for (const size_t band : selected_band) {
            juce::ValueTree filter{juce::Identifier{"filter" + std::to_string(i)}};
            tree.addChild(filter, i, nullptr);
            i += 1;
            const auto band_s = std::to_string(band);
            for (auto& para_ID : kIDs) {
                filter.setProperty(para_ID,
                                   getValue(p_ref_.parameters_, para_ID + band_s),
                                   nullptr);
            }
        }

        juce::SystemClipboard::copyTextToClipboard(tree.toXmlString());
    }

    void RightClickPanel::pasteBand() {
        const auto tree = juce::ValueTree::fromXml(juce::SystemClipboard::getTextFromClipboard());
        if (!tree.hasType("filter_info")) { return; }

        setVisible(false);

        base_.getSelectedBandSet().deselectAll();
        for (size_t i = 0; i < zlp::kBandNum; ++i) {
            const auto filter = tree.getChildWithName(juce::Identifier{"filter" + std::to_string(i)});
            if (!filter.isValid()) { return; }

            const size_t band = band_helper::findOffBand(p_ref_);
            if (band == zlp::kBandNum) { return; }

            const auto band_s = std::to_string(band);
            for (auto& para_ID : kIDs) {
                if (filter.hasProperty(para_ID)) {
                    auto* para = p_ref_.parameters_.getParameter(para_ID + band_s);
                    updateValue(para, para->convertTo0to1(filter.getProperty(para_ID)));
                }
            }
            base_.getSelectedBandSet().addToSelection(band);
            base_.setSelectedBand(band);
        }
    }
}
