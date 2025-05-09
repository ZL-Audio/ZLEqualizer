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
    class CollisionBox final : public juce::Component, private juce::ValueTree::Listener {
    public:
        explicit CollisionBox(juce::AudioProcessorValueTreeState &parametersNA,
                              zlgui::UIBase &base)
            : parametersNARef(parametersNA),
              uiBase(base),
              collisionC("DET:", zlstate::conflictON::choices, uiBase, zlgui::multilingual::labels::collisionDET),
              strengthS("Strength", uiBase, zlgui::multilingual::labels::collisionStrength),
              scaleS("Scale", uiBase, zlgui::multilingual::labels::collisionScale) {
            collisionC.getLabelLAF().setFontScale(1.5f);
            collisionC.setLabelScale(.5f);
            collisionC.setLabelPos(zlgui::ClickCombobox::left);
            addAndMakeVisible(collisionC);
            for (auto &c: {&strengthS, &scaleS}) {
                c->setPadding(uiBase.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({&collisionC.getCompactBox().getBox()},
                   {zlstate::conflictON::ID},
                   parametersNARef, boxAttachments);
            attach({&strengthS.getSlider(), &scaleS.getSlider()},
                   {
                       zlstate::conflictStrength::ID,
                       zlstate::conflictScale::ID
                   },
                   parametersNARef, sliderAttachments);
            setBufferedToImage(true);

            uiBase.getBoxTree().addListener(this);
        }

        ~CollisionBox() override {
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
            const auto buttonHeight = static_cast<int>(buttonHeightP * uiBase.getFontSize());
            const auto buttonWidth = static_cast<int>(uiBase.getFontSize() * 2.5);
            const auto boxHeight = juce::roundToInt(boxHeightP * uiBase.getFontSize());
            return {buttonWidth * 3 + padSize * 2, buttonHeight * 2 + boxHeight + padSize};
        }

        void resized() override {
            for (auto &c: {&strengthS, &scaleS}) {
                c->setPadding(std::round(uiBase.getFontSize() * 0.5f),
                              std::round(uiBase.getFontSize() * 0.6f));
            }

            const auto padSize = juce::roundToInt(uiBase.getFontSize() * 0.25f);
            const auto buttonHeight = static_cast<int>(buttonHeightP * uiBase.getFontSize());

            auto bound = getLocalBounds();
            bound = juce::Rectangle<int>(bound.getX() + padSize, bound.getY(),
                                         bound.getWidth() - padSize * 2, bound.getHeight() - padSize);

            scaleS.setBounds(bound.removeFromBottom(buttonHeight));
            strengthS.setBounds(bound.removeFromBottom(buttonHeight));
            collisionC.setBounds(bound);
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlgui::UIBase &uiBase;

        zlgui::ClickCombobox collisionC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        zlgui::CompactLinearSlider strengthS, scaleS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};

        void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                      const juce::Identifier &property) override {
            juce::ignoreUnused(treeWhosePropertyHasChanged);
            if (uiBase.isBoxProperty(zlgui::boxIdx::collisionBox, property)) {
                const auto f = static_cast<bool>(uiBase.getBoxProperty(zlgui::boxIdx::collisionBox));
                setVisible(f);
            }
        }
    };
}
