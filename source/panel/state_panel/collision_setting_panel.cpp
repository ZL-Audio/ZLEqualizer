// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "collision_setting_panel.hpp"
#include "../../state/state.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {


    CollisionSettingPanel::CollisionSettingPanel(PluginProcessor &p,
                                               zlInterface::UIBase &base)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Collision", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        name.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(name);
        setBufferedToImage(true);
    }

    CollisionSettingPanel::~CollisionSettingPanel() = default;

    void CollisionSettingPanel::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
    }

    void CollisionSettingPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        if (uiBase.getBoxProperty(zlInterface::boxIdx::collisionBox)) {
            uiBase.setBoxProperty(zlInterface::boxIdx::collisionBox, false);
        } else {
            uiBase.openOneBox(zlInterface::boxIdx::collisionBox);
        }
    }

    void CollisionSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }
} // zlPanel
