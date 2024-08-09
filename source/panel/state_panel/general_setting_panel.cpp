// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "general_setting_panel.hpp"

#include "../panel_definitons.hpp"

namespace zlPanel {
    class GeneralCallOutBox final : public juce::Component {
    public:
        explicit GeneralCallOutBox(juce::AudioProcessorValueTreeState &parameters,
                                   zlInterface::UIBase &base)
            : parametersRef(parameters),
              uiBase(base),
              filterStructure("", zlDSP::filterStructure::choices, uiBase),
              zeroLATC("Zero LAT:", zlDSP::zeroLatency::choices, uiBase),
              dynLinkC("Dyn Link:", zlDSP::dynLink::choices, uiBase) {
            for (auto &c: {&filterStructure}) {
                addAndMakeVisible(c);
            }
            for (auto &c: {&zeroLATC, &dynLinkC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.625f);
                c->setLabelPos(zlInterface::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            attach({
                       &filterStructure.getBox(), &zeroLATC.getCompactBox().getBox(), &dynLinkC.getCompactBox().getBox()
                   },
                   {
                       zlDSP::filterStructure::ID, zlDSP::zeroLatency::ID, zlDSP::dynLink::ID
                   },
                   parametersRef, boxAttachments);
        }

        ~GeneralCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(44)), Track(Fr(44)), Track(Fr(44))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(filterStructure).withArea(1, 1),
                juce::GridItem(zeroLATC).withArea(2, 1),
                juce::GridItem(dynLinkC).withArea(3, 1),
            };
            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .4125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactCombobox filterStructure;
        zlInterface::ClickCombobox zeroLATC, dynLinkC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;
    };

    GeneralSettingPanel::GeneralSettingPanel(PluginProcessor &p,
                                             zlInterface::UIBase &base)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("General", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
    }

    GeneralSettingPanel::~GeneralSettingPanel() {
        name.setLookAndFeel(nullptr);
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->setLookAndFeel(nullptr);
        }
    }

    void GeneralSettingPanel::paint(juce::Graphics &g) {
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

    void GeneralSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void GeneralSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void GeneralSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<GeneralCallOutBox>(parametersRef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 10.f),
                         juce::roundToInt(uiBase.getFontSize() * 6.6f));//4.3525f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getBounds(),
                                                           getParentComponent()->getParentComponent());
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
        boxPointer = &box;
    }
} // zlPanel
