// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "output_setting_panel.hpp"
#include "../../dsp/dsp.hpp"

namespace zlPanel {
    class OutputCallOutBox final : public juce::Component {
    public:
        explicit OutputCallOutBox(PluginProcessor &p,
                                  zlInterface::UIBase &base)
            : processorRef(p), parametersRef(p.parameters),
              uiBase(base),
              phaseC("phase", uiBase, zlInterface::multilingual::labels::phaseFlip),
              agcC("A", uiBase, zlInterface::multilingual::labels::autoGC),
              lmC("L", uiBase, zlInterface::multilingual::labels::loudnessMatch),
              scaleS("Scale", uiBase, zlInterface::multilingual::labels::scale),
              outGainS("Out Gain", uiBase, zlInterface::multilingual::labels::outputGain),
              phaseDrawable(
                  juce::Drawable::createFromImageData(BinaryData::fadphase_svg,
                                                      BinaryData::fadphase_svgSize)),
              agcDrawable(juce::Drawable::createFromImageData(BinaryData::autogaincompensation_svg,
                                                              BinaryData::autogaincompensation_svgSize)),
              lmDrawable(juce::Drawable::createFromImageData(BinaryData::loudnessmatch_svg,
                                                             BinaryData::loudnessmatch_svgSize)),
              agcUpdater(p.parameters, zlDSP::autoGain::ID),
              gainUpdater(p.parameters, zlDSP::outputGain::ID) {
            setBufferedToImage(true);
            phaseC.setDrawable(phaseDrawable.get());
            agcC.setDrawable(agcDrawable.get());
            lmC.setDrawable(lmDrawable.get());

            for (auto &c: {&phaseC, &agcC, &lmC}) {
                c->getLAF().enableShadow(false);
                c->getLAF().setShrinkScale(0.f);
                addAndMakeVisible(c);
            }
            for (auto &c: {&scaleS, &outGainS}) {
                c->setPadding(uiBase.getFontSize() * .5f, 0.f);
                addAndMakeVisible(c);
            }
            attach({&phaseC.getButton(), &agcC.getButton(), &lmC.getButton()},
                   {zlDSP::phaseFlip::ID, zlDSP::autoGain::ID, zlDSP::loudnessMatcherON::ID},
                   parametersRef, buttonAttachments);
            attach({&scaleS.getSlider(), &outGainS.getSlider()},
                   {zlDSP::scale::ID, zlDSP::outputGain::ID},
                   parametersRef, sliderAttachments);

            lmC.getButton().onClick = [this]() {
                if (!lmC.getButton().getToggleState()) {
                    const auto newGain = -processorRef.getController().getLoudnessMatcherDiff();
                    agcUpdater.updateSync(0.f);
                    gainUpdater.updateSync(zlDSP::outputGain::convertTo01(static_cast<float>(newGain)));
                }
            };
        }

        ~OutputCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50)), Track(Fr(50)), Track(Fr(50))};

            grid.items = {
                juce::GridItem(scaleS).withArea(1, 1, 2, 4),
                juce::GridItem(phaseC).withArea(2, 1),
                juce::GridItem(agcC).withArea(2, 2),
                juce::GridItem(lmC).withArea(2, 3),
                juce::GridItem(outGainS).withArea(3, 1, 4, 4)
            };

            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .2125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;

        zlInterface::CompactButton phaseC, agcC, lmC;
        juce::OwnedArray<zlInterface::ButtonCusAttachment<false> > buttonAttachments{};

        zlInterface::CompactLinearSlider scaleS, outGainS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments{};

        const std::unique_ptr<juce::Drawable> phaseDrawable;
        const std::unique_ptr<juce::Drawable> agcDrawable;
        const std::unique_ptr<juce::Drawable> lmDrawable;

        zlChore::ParaUpdater agcUpdater, gainUpdater;
    };

    OutputSettingPanel::OutputSettingPanel(PluginProcessor &p,
                                           zlInterface::UIBase &base)
        : processorRef(p),
          parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          currentScale(*parametersRef.getRawParameterValue(zlDSP::scale::ID)),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        lmPara = parametersRef.getParameter(zlDSP::loudnessMatcherON::ID);
        lookAndFeelChanged();
    }

    OutputSettingPanel::~OutputSettingPanel() {
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->dismiss();
        }
    }

    void OutputSettingPanel::paint(juce::Graphics &g) {
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        g.fillPath(backgroundPath);

        g.setFont(uiBase.getFontSize() * 1.375f);
        if (showGain) {
            g.setColour(uiBase.getColourByIdx(zlInterface::gainColour));
            g.drawText(gainString, gainBound, juce::Justification::centred);
            g.drawText(scaleString, scaleBound, juce::Justification::centred);
        } else {
            g.setColour(uiBase.getTextColor());
            g.drawText("Output", getLocalBounds().toFloat(), juce::Justification::centred);
        }
    }

    void OutputSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void OutputSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<OutputCallOutBox>(processorRef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 7.5f),
                         juce::roundToInt(uiBase.getFontSize() * 7.75f));

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
            if (lmPara->getValue() < .5f) {
                if (currentGain <= 0.04) {
                    gainString = juce::String(currentGain, 1, false);
                } else {
                    gainString = "+" + juce::String(currentGain, 1, false);
                }
            } else {
                gainString = "L";
            }
            scaleString = juce::String(static_cast<int>(std::round(currentScale.load()))) + "%";
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
            repaint();
        }
    }

    void OutputSettingPanel::resized() {
        gainBound = getLocalBounds().toFloat();
        scaleBound = gainBound.removeFromRight(gainBound.getWidth() * .5f);
        const auto bound = getLocalBounds().toFloat();
        backgroundPath.clear();
        backgroundPath.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                           uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                           false, false, true, true);
    }
} // zlPanel
