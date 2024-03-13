// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "output_setting_panel.hpp"
#include "../../dsp/dsp.hpp"

namespace zlPanel {
    class OutputCallOutBox final : public juce::Component {
    public:
        explicit OutputCallOutBox(juce::AudioProcessorValueTreeState &parameters,
                                  zlInterface::UIBase &base)
            : parametersRef(parameters),
              uiBase(base.getFontSize(), base.getStyle()),
              effectC("ALL:", zlDSP::effectON::choices, uiBase),
              scaleS("Scale", uiBase),
              outGainS("Out Gain", uiBase) {
            effectC.getLabelLAF().setFontScale(1.5f);
            effectC.setLabelScale(.5f);
            effectC.setLabelPos(zlInterface::ClickCombobox::left);
            addAndMakeVisible(effectC);
            for (auto &c: {&scaleS, &outGainS}) {
                c->setPadding(uiBase.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({&effectC.getCompactBox().getBox()},
                   {zlDSP::effectON::ID},
                   parametersRef, boxAttachments);
            attach({&scaleS.getSlider(), &outGainS.getSlider()},
                   {zlDSP::scale::ID, zlDSP::outputGain::ID},
                   parametersRef, sliderAttachments);
        }

        ~OutputCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(44)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(effectC).withArea(1, 1),
                juce::GridItem(scaleS).withArea(2, 1),
                juce::GridItem(outGainS).withArea(3, 1)
            };

            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .4125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase uiBase;

        zlInterface::ClickCombobox effectC;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};

        zlInterface::CompactLinearSlider scaleS, outGainS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};
    };

    OutputSettingPanel::OutputSettingPanel(juce::AudioProcessorValueTreeState &parameters,
                                           juce::AudioProcessorValueTreeState &parametersNA,
                                           zlInterface::UIBase &base)
        : parametersRef(parameters),
          parametersNARef(parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Output", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
        // setBufferedToImage(true);
    }

    OutputSettingPanel::~OutputSettingPanel() {
        name.setLookAndFeel(nullptr);
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->dismiss();
        }
    }

    void OutputSettingPanel::paint(juce::Graphics &g) {
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

    void OutputSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void OutputSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void OutputSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<OutputCallOutBox>(parametersRef, uiBase);
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
