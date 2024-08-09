// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "conflict_setting_panel.hpp"
#include "../../state/state.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    class ConflictCallOutBox final : public juce::Component {
    public:
        explicit ConflictCallOutBox(juce::AudioProcessorValueTreeState &parametersNA,
                                    zlInterface::UIBase &base)
            : parametersNARef(parametersNA),
              uiBase(base),
              conflictC("DET:", zlState::conflictON::choices, uiBase),
              strengthS("Strength", uiBase),
              scaleS("Scale", uiBase) {
            conflictC.getLabelLAF().setFontScale(1.5f);
            conflictC.setLabelScale(.5f);
            conflictC.setLabelPos(zlInterface::ClickCombobox::left);
            addAndMakeVisible(conflictC);
            for (auto &c: {&strengthS, &scaleS}) {
                c->setPadding(uiBase.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({&conflictC.getCompactBox().getBox()},
                   {zlState::conflictON::ID},
                   parametersNARef, boxAttachments);
            attach({&strengthS.getSlider(), &scaleS.getSlider()},
                   {
                       zlState::conflictStrength::ID,
                       zlState::conflictScale::ID
                   },
                   parametersNARef, sliderAttachments);
        }

        ~ConflictCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(44)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(conflictC).withArea(1, 1),
                juce::GridItem(strengthS).withArea(2, 1),
                juce::GridItem(scaleS).withArea(3, 1)
            };

            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .4125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase &uiBase;

        zlInterface::ClickCombobox conflictC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        zlInterface::CompactLinearSlider strengthS, scaleS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};
    };

    ConflictSettingPanel::ConflictSettingPanel(PluginProcessor &p,
                                               zlInterface::UIBase &base)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Collision", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
    }

    ConflictSettingPanel::~ConflictSettingPanel() {
        name.setLookAndFeel(nullptr);
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->dismiss();
        }
    }

    void ConflictSettingPanel::paint(juce::Graphics &g) {
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

    void ConflictSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void ConflictSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void ConflictSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<ConflictCallOutBox>(parametersNARef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 7.5f),
                         juce::roundToInt(uiBase.getFontSize() * 8.4f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getBounds(),
                                                           getParentComponent()->getParentComponent());
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
        boxPointer = &box;
    }
} // zlPanel
