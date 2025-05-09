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
    class GeneralBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit GeneralBox(juce::AudioProcessorValueTreeState &parameters,
                            zlgui::UIBase &base)
            : parameters_ref(parameters),
              uiBase(base),
              filterStructure("", zlp::filterStructure::choices, uiBase,
                              zlgui::multilingual::labels::filterStructure,
                              {
                                  zlgui::multilingual::labels::minimumPhase,
                                  zlgui::multilingual::labels::stateVariable,
                                  zlgui::multilingual::labels::parallelPhase,
                                  zlgui::multilingual::labels::matchedPhase,
                                  zlgui::multilingual::labels::mixedPhase,
                                  zlgui::multilingual::labels::linearPhase
                              }),
              zeroLATC("Zero LAT:", zlp::zeroLatency::choices, uiBase,
                       zlgui::multilingual::labels::zeroLatency) {
            for (auto &c: {&filterStructure}) {
                addAndMakeVisible(c);
            }
            for (auto &c: {&zeroLATC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.625f);
                c->setLabelPos(zlgui::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            attach({
                       &filterStructure.getBox(), &zeroLATC.getCompactBox().getBox()
                   },
                   {
                       zlp::filterStructure::ID, zlp::zeroLatency::ID
                   },
                   parameters_ref, boxAttachments);
            setBufferedToImage(true);

            uiBase.getBoxTree().addListener(this);
        }

        ~GeneralBox() override {
            uiBase.getBoxTree().removeListener(this);
        }

        void paint(juce::Graphics &g) override {
            juce::Path path;
            const auto bound = getLocalBounds().toFloat();
            path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                     std::round(uiBase.getFontSize() * .25f),
                                     std::round(uiBase.getFontSize() * .25f),
                                     false, false, true, true);
            g.setColour(uiBase.getBackgroundColor());
            g.fillPath(path);
        }

        juce::Rectangle<int> getIdealBound() const {
            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            return {static_cast<int>(uiBase.getFontSize() * 10) + padSize * 2, boxHeight * 2 + padSize};
        }

        void resized() override {
            auto bound = getLocalBounds();
            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            filterStructure.setBounds(bound.removeFromTop(boxHeight));
            zeroLATC.setBounds(bound.removeFromTop(boxHeight));
        }

    private:
        juce::AudioProcessorValueTreeState &parameters_ref;
        zlgui::UIBase &uiBase;

        zlgui::CompactCombobox filterStructure;
        zlgui::ClickCombobox zeroLATC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (uiBase.isBoxProperty(zlgui::boxIdx::generalBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlgui::boxIdx::generalBox));
                setVisible(f);
            }
        }
    };
}
