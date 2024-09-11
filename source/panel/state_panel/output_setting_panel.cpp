// Copyright (C) 2024 - zsliu98
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
              uiBase(base),
              effectC("all", uiBase),
              phaseC("phase", uiBase),
              sgcC("S", uiBase),
              agcC("A", uiBase),
              scaleS("Scale", uiBase),
              outGainS("Out Gain", uiBase),
              effectDrawable(
                  juce::Drawable::createFromImageData(BinaryData::fadpowerswitch_svg,
                                                      BinaryData::fadpowerswitch_svgSize)),
              phaseDrawable(
                  juce::Drawable::createFromImageData(BinaryData::fadphase_svg,
                                                      BinaryData::fadphase_svgSize)) {
            effectC.setDrawable(effectDrawable.get());
            phaseC.setDrawable(phaseDrawable.get());

            effectC.getLAF().enableShadow(false);
            addAndMakeVisible(effectC);
            for (auto &c: {&phaseC, &sgcC, &agcC}) {
                c->getLAF().setLabelScale(2.f);
                c->getLAF().enableShadow(false);
                c->getLAF().setShrinkScale(.0f);
                addAndMakeVisible(c);
            }
            for (auto &c: {&scaleS, &outGainS}) {
                c->setPadding(uiBase.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({
                       &effectC.getButton(),
                       &phaseC.getButton(),
                       &sgcC.getButton(),
                       &agcC.getButton()
                   },
                   {zlDSP::effectON::ID, zlDSP::phaseFlip::ID, zlDSP::staticAutoGain::ID, zlDSP::autoGain::ID},
                   parametersRef, buttonAttachments);
            attach({&scaleS.getSlider(), &outGainS.getSlider()},
                   {zlDSP::scale::ID, zlDSP::outputGain::ID},
                   parametersRef, sliderAttachments);
        }

        ~OutputCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(70)), Track(Fr(60)), Track(Fr(70)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50)), Track(Fr(50))};

            grid.items = {
                juce::GridItem(effectC).withArea(1, 1),
                juce::GridItem(phaseC).withArea(1, 2),
                juce::GridItem(scaleS).withArea(2, 1, 3, 3),
                juce::GridItem(sgcC).withArea(3, 1),
                juce::GridItem(agcC).withArea(3, 2),
                juce::GridItem(outGainS).withArea(4, 1, 5, 3)
            };

            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .2125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactButton effectC, phaseC, sgcC, agcC;
        juce::OwnedArray<zlInterface::ButtonCusAttachment<true> > buttonAttachments{};

        zlInterface::CompactLinearSlider scaleS, outGainS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};

        const std::unique_ptr<juce::Drawable> effectDrawable, phaseDrawable;
    };

    OutputSettingPanel::OutputSettingPanel(PluginProcessor &p,
                                           zlInterface::UIBase &base)
        : processorRef(p),
          parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        lookAndFeelChanged();
    }

    OutputSettingPanel::~OutputSettingPanel() {
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

        if (showGain) {
            g.setColour(uiBase.getColourByIdx(zlInterface::gainColour));
        } else {
            g.setColour(uiBase.getTextColor());
        }
        g.setFont(uiBase.getFontSize() * 1.375f);
        g.drawText(displayString, bound, juce::Justification::centred);
    }

    void OutputSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void OutputSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<OutputCallOutBox>(parametersRef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 7.5f),
                         juce::roundToInt(uiBase.getFontSize() * 11.2f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getBounds(),
                                                           getParentComponent()->getParentComponent());
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
        boxPointer = &box;
    }

    void OutputSettingPanel::timerCallback() {
        const auto currentGain = processorRef.getController().getGainCompensation();
        showGain = !showGain;
        if (showGain) {
            juce::String gainString;
            if (currentGain <= -10.0) {
                gainString = juce::String(currentGain, 1, false);
            } else if (currentGain < 0.0) {
                gainString = juce::String(currentGain, 1, false);
            } else if (currentGain < 10.0) {
                gainString = "+" + juce::String(currentGain, 1, false);
            } else {
                gainString = "+" + juce::String(currentGain, 1, false);
            }
            displayString = gainString;
        } else {
            displayString = "Output";
        }
        repaint();
    }

    void OutputSettingPanel::lookAndFeelChanged() {
        if (uiBase.getColourByIdx(zlInterface::gainColour).getAlpha() > juce::uint8(0)) {
            showGain = true;
            startTimer(1500);
        } else {
            stopTimer();
            showGain = false;
            displayString = "Output";
            repaint();
        }
    }
} // zlPanel
