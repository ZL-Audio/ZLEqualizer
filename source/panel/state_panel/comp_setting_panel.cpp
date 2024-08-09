// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "comp_setting_panel.hpp"
#include "../panel_definitons.hpp"
#include "../../dsp/dsp.hpp"

namespace zlPanel {
    class CompCallOutBox final : public juce::Component {
    public:
        explicit CompCallOutBox(juce::AudioProcessorValueTreeState &parameters,
                                zlInterface::UIBase &base)
            : parametersRef(parameters),
              uiBase(base),
              lookaheadS("Lookahead", uiBase),
              rmsS("RMS", uiBase),
              smoothS("Smooth", uiBase),
              dynHQC("HQ:", zlDSP::dynHQ::choices, uiBase) {
            for (auto &c: {&lookaheadS, &rmsS, &smoothS}) {
                c->setPadding(uiBase.getFontSize() * .5f, 0.01f);
                addAndMakeVisible(c);
            }
            attach({
                       &lookaheadS.getSlider(), &rmsS.getSlider(), &smoothS.getSlider()
                   },
                   {
                       zlDSP::dynLookahead::ID, zlDSP::dynRMS::ID, zlDSP::dynSmooth::ID
                   },
                   parametersRef, sliderAttachments);
            for (auto &c: {&dynHQC}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlInterface::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            attach({
                       &dynHQC.getCompactBox().getBox(),
                   },
                   {
                       zlDSP::dynHQ::ID
                   },
                   parametersRef, boxAttachments);
        }

        ~CompCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(44))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(lookaheadS).withArea(1, 1),
                juce::GridItem(rmsS).withArea(2, 1),
                juce::GridItem(smoothS).withArea(3, 1),
                juce::GridItem(dynHQC).withArea(4, 1)
            };

            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .4125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactLinearSlider lookaheadS, rmsS, smoothS;
        zlInterface::ClickCombobox dynHQC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};
    };

    CompSettingPanel::CompSettingPanel(PluginProcessor &p,
                                       zlInterface::UIBase &base)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Dynamic", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
    }

    CompSettingPanel::~CompSettingPanel() {
        name.setLookAndFeel(nullptr);
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->dismiss();
        }
    }

    void CompSettingPanel::paint(juce::Graphics &g) {
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

    void CompSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void CompSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void CompSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<CompCallOutBox>(parametersRef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 7.5f),
                         juce::roundToInt(uiBase.getFontSize() * 11.5201933f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getBounds(),
                                                           getParentComponent()->getParentComponent());
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
        boxPointer = &box;
    }
} // zlPanel
