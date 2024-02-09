// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_setting_panel.hpp"

namespace zlPanel {
    class FFTCallOutBox final : public juce::Component {
    public:
        explicit FFTCallOutBox(juce::AudioProcessorValueTreeState &parametersNA,
                               zlInterface::UIBase &base)
            : parametersNARef(parametersNA),
              uiBase(base.getFontSize(), base.getStyle()),
              fftPreON("", zlState::fftPreON::choices, uiBase),
              fftPostON("", zlState::fftPostON::choices, uiBase),
              fftSideON("", zlState::fftSideON::choices, uiBase),
              ffTSpeed("", zlState::ffTSpeed::choices, uiBase),
              fftTilt("", zlState::ffTTilt::choices, uiBase),
              preLabel("", "Pre:"),
              postLabel("", "Post:"),
              sideLabel("", "Side:"),
              labelLAF(uiBase) {
            for (auto &c: {&fftPreON, &fftPostON, &fftSideON, &ffTSpeed, &fftTilt}) {
                addAndMakeVisible(c);
            }
            labelLAF.setFontScale(1.5f);
            labelLAF.setJustification(juce::Justification::centredRight);
            for (auto &l: {&preLabel, &postLabel, &sideLabel}) {
                l->setLookAndFeel(&labelLAF);
                addAndMakeVisible(l);
            }
            attach({
                       &fftPreON.getBox(), &fftPostON.getBox(), &fftSideON.getBox(),
                       &ffTSpeed.getBox(), &fftTilt.getBox()
                   },
                   {
                       zlState::fftPreON::ID, zlState::fftPostON::ID, zlState::fftSideON::ID,
                       zlState::ffTSpeed::ID, zlState::ffTTilt::ID
                   },
                   parametersNARef, boxAttachments);
            setBufferedToImage(true);
        }

        ~FFTCallOutBox() override {
            for (auto &l: {&preLabel, &postLabel, &sideLabel}) {
                l->setLookAndFeel(nullptr);
            }
        }

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50)), Track(Fr(50))};

            grid.items = {
                juce::GridItem(preLabel).withArea(1, 1),
                juce::GridItem(postLabel).withArea(2, 1),
                juce::GridItem(sideLabel).withArea(3, 1),
                juce::GridItem(fftPreON).withArea(1, 2),
                juce::GridItem(fftPostON).withArea(2, 2),
                juce::GridItem(fftSideON).withArea(3, 2),
                juce::GridItem(ffTSpeed).withArea(4, 1, 5, 3),
                juce::GridItem(fftTilt).withArea(5, 1, 6, 3)
            };

            const auto bound = getLocalBounds().toFloat();
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase uiBase;

        zlInterface::CompactCombobox fftPreON, fftPostON, fftSideON, ffTSpeed, fftTilt;
        juce::Label preLabel, postLabel, sideLabel;
        zlInterface::NameLookAndFeel labelLAF;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;
    };

    FFTSettingPanel::FFTSettingPanel(juce::AudioProcessorValueTreeState &parameters,
                                     juce::AudioProcessorValueTreeState &parametersNA,
                                     zlInterface::UIBase &base)
        : parametersRef(parameters),
          parametersNARef(parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          // drawable(juce::Drawable::createFromImageData(BinaryData::fadwaveform_svg, BinaryData::fadwaveform_svgSize)),
          // button(drawable.get(), base),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Analyzer", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
        setBufferedToImage(true);
    }

    FFTSettingPanel::~FFTSettingPanel() {
        name.setLookAndFeel(nullptr);
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->dismiss();
        }
    }

    void FFTSettingPanel::paint(juce::Graphics &g) {
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

    void FFTSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void FFTSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void FFTSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<FFTCallOutBox>(parametersNARef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 7.f),
                         juce::roundToInt(uiBase.getFontSize() * 9.167f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getScreenBounds(),
                                                           nullptr);
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.updatePosition(getScreenBounds(), getTopLevelComponent()->getScreenBounds());
        box.sendLookAndFeelChange();
        boxPointer = &box;
    }
} // zlPanel
