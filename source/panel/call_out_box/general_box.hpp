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

namespace zlPanel {
    class GeneralBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit GeneralBox(juce::AudioProcessorValueTreeState &parameters,
                            zlInterface::UIBase &base)
            : parametersRef(parameters),
              uiBase(base),
              filterStructure("", zlDSP::filterStructure::choices, uiBase,
                              zlInterface::multilingual::labels::filterStructure,
                              {
                                  zlInterface::multilingual::labels::minimumPhase,
                                  zlInterface::multilingual::labels::stateVariable,
                                  zlInterface::multilingual::labels::parallelPhase,
                                  zlInterface::multilingual::labels::matchedPhase,
                                  zlInterface::multilingual::labels::mixedPhase,
                                  zlInterface::multilingual::labels::linearPhase
                              }),
              zeroLATC("Zero LAT:", zlDSP::zeroLatency::choices, uiBase,
                       zlInterface::multilingual::labels::zeroLatency) {
            for (auto &c: {&filterStructure}) {
                addAndMakeVisible(c);
            }
            for (auto &c: {&zeroLATC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.625f);
                c->setLabelPos(zlInterface::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            attach({
                       &filterStructure.getBox(), &zeroLATC.getCompactBox().getBox()
                   },
                   {
                       zlDSP::filterStructure::ID, zlDSP::zeroLatency::ID
                   },
                   parametersRef, boxAttachments);
            setBufferedToImage(true);

            uiBase.getBoxTree().addListener(this);
        }

        ~GeneralBox() override {
            uiBase.getBoxTree().removeListener(this);
        }

        void paint(juce::Graphics &g) override {
            g.fillAll(uiBase.getBackgroundColor());
        }

        juce::Rectangle<int> getIdealBound() const {
            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            return {static_cast<int>(uiBase.getFontSize() * 10), boxHeight * 2 + padSize};
        }

        void resized() override {
            auto bound = getLocalBounds();

            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            filterStructure.setBounds(bound.removeFromTop(boxHeight));
            zeroLATC.setBounds(bound.removeFromTop(boxHeight));
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactCombobox filterStructure;
        zlInterface::ClickCombobox zeroLATC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (uiBase.isBoxProperty(zlInterface::boxIdx::generalBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlInterface::boxIdx::generalBox));
                setVisible(f);
            }
        }
    };
}
