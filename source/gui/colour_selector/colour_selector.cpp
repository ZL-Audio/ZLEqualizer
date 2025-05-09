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
            : selector(selectorFlags,
                       juce::roundToInt(base.getFontSize() * 0.5f),
                       juce::roundToInt(base.getFontSize() * 0.33f)),
              uiBase(base) {
            selector.setColour(juce::ColourSelector::ColourIds::backgroundColourId, uiBase.getBackgroundColor());
            addAndMakeVisible(selector);
        }

        ~SelectorBox() override {
            setLookAndFeel(nullptr);
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(
                bound.getWidth() - uiBase.getFontSize() * .5f,
                bound.getHeight() - uiBase.getFontSize() * .5f);
            selector.setBounds(bound.toNearestInt());
        }

        juce::ColourSelector &getSelector() { return selector; }

    private:
        juce::ColourSelector selector;
        zlgui::UIBase &uiBase;
    };

    ColourSelector::ColourSelector(zlgui::UIBase &base, juce::Component &parent,
                                   const float widthS, const float heightS)
        : uiBase(base), laf(uiBase), parentC(parent),
          selectorWidthS(widthS), selectorHeightS(heightS) {
    }

    void ColourSelector::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getTextColor().withAlpha(.875f));
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - uiBase.getFontSize() * .375f,
                                            bound.getHeight() - uiBase.getFontSize() * .375f);
        g.setColour(uiBase.getBackgroundColor());
        g.fillRect(bound);
        g.setColour(colour);
        g.fillRect(bound);
    }

    void ColourSelector::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        auto colourSelector = std::make_unique<SelectorBox>(
            juce::ColourSelector::ColourSelectorOptions::showColourspace, uiBase);
        colourSelector->getSelector().setCurrentColour(colour);
        colourSelector->getSelector().addChangeListener(this);
        colourSelector->setSize(juce::roundToInt(selectorWidthS * uiBase.getFontSize()),
                                juce::roundToInt(selectorHeightS * uiBase.getFontSize()));
        auto &box = juce::CallOutBox::launchAsynchronously(std::move(colourSelector),
                                                           parentC.getLocalArea(this, getLocalBounds()),
                                                           &parentC);
        box.setLookAndFeel(&laf);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
    }

    void ColourSelector::changeListenerCallback(juce::ChangeBroadcaster *source) {
        if (const auto *cs = dynamic_cast<juce::ColourSelector *>(source)) {
            colour = cs->getCurrentColour().withAlpha(colour.getAlpha());
            repaint();
        }
    }
} // zlgui
