// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_selector.hpp"

namespace zlgui {
    class SelectorBox final : public juce::Component {
    public:
        explicit SelectorBox(const int selectorFlags, zlgui::UIBase &base)
            : selector_(selectorFlags,
                       juce::roundToInt(base.getFontSize() * 0.5f),
                       juce::roundToInt(base.getFontSize() * 0.33f)),
              ui_base_(base) {
            selector_.setColour(juce::ColourSelector::ColourIds::backgroundColourId, ui_base_.getBackgroundColor());
            addAndMakeVisible(selector_);
        }

        ~SelectorBox() override {
            setLookAndFeel(nullptr);
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(
                bound.getWidth() - ui_base_.getFontSize() * .5f,
                bound.getHeight() - ui_base_.getFontSize() * .5f);
            selector_.setBounds(bound.toNearestInt());
        }

        juce::ColourSelector &getSelector() { return selector_; }

    private:
        juce::ColourSelector selector_;
        zlgui::UIBase &ui_base_;
    };

    ColourSelector::ColourSelector(zlgui::UIBase &base, juce::Component &parent,
                                   const float width_s, const float height_s)
        : ui_base_(base), laf_(ui_base_), parent_ref_(parent),
          selector_width_s_(width_s), selector_height_s_(height_s) {
    }

    void ColourSelector::paint(juce::Graphics &g) {
        g.fillAll(ui_base_.getTextColor().withAlpha(.875f));
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - ui_base_.getFontSize() * .375f,
                                            bound.getHeight() - ui_base_.getFontSize() * .375f);
        g.setColour(ui_base_.getBackgroundColor());
        g.fillRect(bound);
        g.setColour(colour_);
        g.fillRect(bound);
    }

    void ColourSelector::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        auto colour_selector = std::make_unique<SelectorBox>(
            juce::ColourSelector::ColourSelectorOptions::showColourspace, ui_base_);
        colour_selector->getSelector().setCurrentColour(colour_);
        colour_selector->getSelector().addChangeListener(this);
        colour_selector->setSize(juce::roundToInt(selector_width_s_ * ui_base_.getFontSize()),
                                juce::roundToInt(selector_height_s_ * ui_base_.getFontSize()));
        auto &box = juce::CallOutBox::launchAsynchronously(std::move(colour_selector),
                                                           parent_ref_.getLocalArea(this, getLocalBounds()),
                                                           &parent_ref_);
        box.setLookAndFeel(&laf_);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
    }

    void ColourSelector::changeListenerCallback(juce::ChangeBroadcaster *source) {
        if (const auto *cs = dynamic_cast<juce::ColourSelector *>(source)) {
            colour_ = cs->getCurrentColour().withAlpha(colour_.getAlpha());
            repaint();
        }
    }
} // zlgui
