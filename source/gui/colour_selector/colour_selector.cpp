// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_selector.hpp"

namespace zlInterface {
    class SelectorBox final : public juce::Component {
    public:
        explicit SelectorBox(const int flags, zlInterface::UIBase &base)
            : selector(flags,
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
        zlInterface::UIBase &uiBase;
    };

    ColourSelector::ColourSelector(zlInterface::UIBase &base, juce::Component &parent,
                                   const float widthS, const float heightS)
        : uiBase(base), laf(uiBase), parentC(parent),
          selectorWidthS(widthS), selectorHeightS(heightS) {
    }

    void ColourSelector::paint(juce::Graphics &g) {
        g.fillAll(colour);
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
} // zlInterface
